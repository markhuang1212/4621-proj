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
#include <zlib.h>

#define SERVER_PORT (8081)
#define LISTENNQ (8)
#define MAXTHREAD (8)
#define CHUNK (16384)
#define true (1)
#define false (0)
#define bool int

const int MAX_REQ_LENGTH = 8192;

sem_t num_of_active_thread;

/**
 * Pipe a file descriptior to another
 * Return 0 on success, -1 on error
 */
int pipe_fd(int src, int dest, time_t timeout)
{
    char buff[CHUNK];
    while (true)
    {
        ssize_t ret = read(src, buff, CHUNK);
        if (ret == 0)
        {
            break;
        }
        if (ret < 0)
        {
            printf("Error: Read fd\n");
            printf("%s\n", strerror(errno));
            return -1;
        }
        ssize_t ret2 = write(dest, buff, ret);
        if (ret2 < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                if (time(NULL) <= timeout)
                {
                    pthread_yield();
                    continue;
                }
            }
            printf("Error: Write fd\n");
            printf("%s\n", strerror(errno));
            return -1;
        }
    }
    return 0;
}

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

int get_content_length(int fd)
{
    struct stat st;
    fstat(fd, &st);
    return st.st_size;
}

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
    time_t timeout = time(NULL) + 30;

    /* get the connection fd */
    int connfd = *(int *)args;

    // process request
    char request[MAX_REQ_LENGTH];
    size_t request_len = 0;

    // read until request completed
    while (true)
    {
        if (request_len >= (size_t)MAX_REQ_LENGTH)
        {
            printf("Error: Request Size Exceed\n");
            exit(1);
        }

        char buffer;
        // read one byte at a time
        ssize_t ret = read(connfd, &buffer, 1);
        if (ret == 0)
        {
            pthread_yield();
            continue;
        }
        if (ret == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                if (time(NULL) <= timeout)
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

    // parse url
    strtok(request, " ");
    char *uri = strtok(NULL, " ");
    printf("URI: %s \n", uri);

    char path[512];
    snprintf(path, 512, "static%s", uri);

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

    int content_length = get_content_length(page_fd);

    char buff[1024];
    snprintf(buff, 1024, "HTTP/1.1 %s                   \r\n"
                         "Content-Type: %s              \r\n"
                         "Cache-Control: max-age=3600   \r\n"
                         "\r\n",
             is404 ? "404 Not Found" : "200 OK",
             extension,
             content_length);

    if (write(connfd, buff, strlen(buff)) < 0)
    {
        printf("Error: Write Socket Header\n");
        exit(1);
    }

    pipe_fd(page_fd, connfd, timeout);
    close(page_fd);

    close(connfd);
    sem_post(&num_of_active_thread);
    printf("==================== Request Ended ====================== \n");
    return NULL;
}

int main()
{
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