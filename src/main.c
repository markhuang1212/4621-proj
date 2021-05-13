/* ONLY support Linux */
#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <time.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

/* Require zlib library */
#include <zlib.h>

#define LISTENNQ (8)
#define MAXTHREAD (8)
#define CHUNK (16384)
#define true (1)
#define false (0)
#define bool int

int SERVER_PORT = 8081;
int MAX_REQ_LENGTH = 8192;

/* Make sure the number of threads does not exceeds a limit */
sem_t num_of_active_thread;

/**
 * Compress the content of a
 * file descriptor, and pipe it to another
 * Return 0 on success, -1 on error
 */
int pipe_fd(int src, int dest)
{
    int ret, flush;
    unsigned int have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION);

    do
    {
        ret = read(src, in, CHUNK);
        if (ret < 0)
        {
            printf("Read File Error\n");
            printf("%s\n", strerror(errno));
            exit(1);
        }
        strm.avail_in = ret;
        strm.next_in = in;
        flush = ret == 0 ? Z_FINISH : Z_NO_FLUSH;

        strm.avail_out = CHUNK;
        strm.next_out = out;
        ret = deflate(&strm, flush);
        have = CHUNK - strm.avail_out;

        ret = write(dest, out, have);
        if (ret < 0)
        {
            printf("Write Socket Error\n");
            printf("%s\n", strerror(errno));
            exit(1);
        }

    } while (flush != Z_FINISH);
    deflateEnd(&strm);

    return 0;
}

/**
 * Determine whether a http GET request has ended.
 */
bool has_request_end(char *request, size_t request_len)
{
    if (request_len >= 4 &&
        request[request_len - 4] == '\r' &&
        request[request_len - 3] == '\n' &&
        request[request_len - 2] == '\r' &&
        request[request_len - 1] == '\n')
        return true;
    else
        return false;
}

/**
 * Transfer file extension to MIME type.
 */
int ext_to_mime(char *buff, size_t buff_len)
{
    if (buff_len <= 20)
    {
        return -1;
    }
    if (strcmp(buff, "html") == 0)
    {
        strcpy(buff, "text/html");
    }
    else if (strcmp(buff, "css") == 0)
    {
        strcpy(buff, "text/css");
    }
    else if (strcmp(buff, "js") == 0)
    {
        strcpy(buff, "text/javascript");
    }
    else if (strcmp(buff, "png") == 0)
    {
        strcpy(buff, "image/png");
    }
    else if (strcmp(buff, "jpg") == 0)
    {
        strcpy(buff, "image/jpg");
    }
    else if (strcmp(buff, "pdf") == 0)
    {
        strcpy(buff, "application/pdf");
    }
    else
    {
        strcpy(buff, "text/plain");
    }
    return 0;
}

void *request_func(void *args)
{
    /* A thread will kill itself if the client is not responsive */
    time_t timeout = time(NULL) + 30;

    /* get the connection fd */
    int connfd = *(int *)args;

    /* process request */
    char request[MAX_REQ_LENGTH];
    size_t request_len = 0;

    /* read until request completed */
    while (true)
    {
        /* if request length exceeds limit, abort. */
        if (request_len >= (size_t)MAX_REQ_LENGTH)
        {
            printf("Error: Request Size Exceed\n");
            close(connfd);
            sem_post(&num_of_active_thread);
            printf("==================== Request Ended ====================== \n");
            return NULL;
        }

        char buffer;
        /* read one byte at a time */
        ssize_t ret = read(connfd, &buffer, 1);

        if (ret == 0) /* nothing to read */
        {
            pthread_yield();
            continue;
        }

        if (ret == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK) /* nothing to read */
            {
                if (time(NULL) <= timeout) /* check timeout */
                {
                    pthread_yield();
                    continue;
                }
            }
            printf("Error: Read Socket\n");
            printf("%s \n", strerror(errno));
            close(connfd);
            sem_post(&num_of_active_thread);
            printf("==================== Request Ended ====================== \n");
            return NULL;
        }
        printf("%c", buffer);
        request[request_len] = buffer;
        request_len++;

        bool is_finished = has_request_end(request, request_len);
        if (is_finished)
            break;
    }

    /* parse url */
    strtok(request, " ");
    char *uri = strtok(NULL, " ");
    printf("URI: %s \n", uri);

    char path[512];
    snprintf(path, 512, "static%s", uri);

    /* parse extension */
    char extension[512]; // extention => mime type
    int s = 0;
    for (int i = strlen(path); i >= 0; i--)
    {
        if (path[i] == '.')
        {
            s = i + 1;
            break;
        }
    }
    strcpy(extension, &path[s]);
    ext_to_mime(extension, 512);

    /**
     * check if the file pointed by `path` exists
     * If not, use 404.html
     */
    bool is404 = false;
    if (strcmp(path, "static/") == 0)
    {
        strcpy(path, "static/index.html");
        strcpy(extension, "text/html");
    }
    else if (access(path, R_OK) != 0)
    {
        is404 = true;
        strcpy(path, "static/404.html");
        strcpy(extension, "text/html");
    }

    int page_fd = open(path, O_RDONLY);
    if (page_fd < 0)
    {
        printf("Read File Error\n");
        exit(1);
    }

    printf("MIME Type: %s \n", extension);

    char buff[1024];
    snprintf(buff, 1024, "HTTP/1.1 %s                   \r\n"
                         "Content-Type: %s              \r\n"
                         "Cache-Control: max-age=3600   \r\n"
                         "Content-Encoding: deflate    \r\n"
                         "\r\n",
             is404 ? "404 Not Found" : "200 OK",
             extension);

    if (write(connfd, buff, strlen(buff)) < 0)
    {
        printf("Error: Write Socket Header\n");
        exit(1);
    }

    pipe_fd(page_fd, connfd);

    close(page_fd);
    close(connfd);
    sem_post(&num_of_active_thread);
    printf("==================== Request Ended ====================== \n");
    return NULL;
}

int main()
{

    char *port_str = getenv("SERVER_PORT");
    if (port_str != NULL)
        SERVER_PORT = atoi(port_str);

    sem_init(&num_of_active_thread, false, MAXTHREAD);

    int listenfd, connfd;

    struct sockaddr_in servaddr, cliaddr;
    socklen_t len = sizeof(struct sockaddr_in);

    char ip_str[INET_ADDRSTRLEN] = {0};

    /* initialize server socket */
    listenfd = socket(AF_INET, SOCK_STREAM, 0); /* SOCK_STREAM : TCP */
    if (listenfd < 0)
    {
        printf("Error: init socket\n");
        return 0;
    }

    /* set tcp socket option */
    int optval = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

    /* initialize server address (IP:port) */
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;  /* IP address: 0.0.0.0 */
    servaddr.sin_port = htons(SERVER_PORT); /* port number */

    /* bind the socket to the server address */
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr)) < 0)
    {
        printf("Error: bind\n");
        return 0;
    }

    if (listen(listenfd, LISTENNQ) < 0)
    {
        printf("Error: listen\n");
        return 0;
    }

    /* keep processing incoming requests */
    while (1)
    {
        /* Wait until there is thread quota */
        sem_wait(&num_of_active_thread);

        /* accept an incoming connection from the remote side */
        connfd = accept4(listenfd, (struct sockaddr *)&cliaddr, &len, SOCK_NONBLOCK);
        if (connfd < 0)
        {
            sem_post(&num_of_active_thread);
            printf("Error: accept\n");
            continue;
        }

        /* print client (remote side) address (IP : port) */
        inet_ntop(AF_INET, &(cliaddr.sin_addr), ip_str, INET_ADDRSTRLEN);
        printf("Incoming connection from %s : %hu with fd: %d\n", ip_str, ntohs(cliaddr.sin_port), connfd);

        /* create dedicate thread to process the request */
        pthread_t tid;
        if (pthread_create(&tid, NULL, request_func, &connfd) != 0)
        {
            sem_post(&num_of_active_thread);
            printf("Error when creating thread %ld\n", tid);
            close(connfd);
            continue;
        }
        pthread_detach(tid);
    }

    return 0;
}