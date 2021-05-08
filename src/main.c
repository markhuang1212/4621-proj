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

#define SERVER_PORT (8081)
#define LISTENNQ (8)
#define MAXTHREAD (8)
#define true (1)
#define false (0)

const int MAX_REQ_LENGTH = 8192;

sem_t num_of_active_thread;

int has_request_end(char *request, size_t request_len)
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

int ext_to_mime(char *buff, size_t buff_len)
{
    if (strcmp(buff, "html") == 0)
    {
        buff = "text/html";
    }
    else if (strcmp(buff, "css") == 0)
    {
        buff = "text/css";
    }
    else if (strcmp(buff, "js") == 0)
    {
        buff = "text/js";
    }
    else if (strcmp(buff, "png") == 0)
    {
        buff = "image/png";
    }
    else
    {
        buff = "text/plain";
    }
}

void *request_func(void *args)
{
    /* get the connection fd */
    int connfd = *(int *)args;

    // process request
    char request[MAX_REQ_LENGTH];
    size_t request_len = 0;

    while (true)
    {
        char buffer;
        ssize_t ret = read(connfd, &buffer, 1);
        if (ret == 0)
        {
            printf("Error: Read\n");
            exit(1);
        }
        request[request_len] = buffer;
        request_len++;
        if (request_len >= MAX_REQ_LENGTH)
        {
            printf("Error: Request Size Exceed\n");
            exit(1);
        }
        int is_finished = has_request_end(request, request_len);
        if (is_finished)
        {
            break;
        }
    }

    strtok(request, " ");
    char *uri = strtok(NULL, " ");
    printf("URI: %s \n", uri);

    if (strcmp(uri, "/") == 0)
    {
        printf("Sending index.html\n");
        int page_fd = open("static/index.html", O_RDONLY);
        struct stat st;
        fstat(page_fd, &st);
        size_t content_length = st.st_size;

        char buff[512];
        snprintf(buff, 512, "HTTP/1.1 200 OK \r\n"
                            "Content-Type: text/html \r\n"
                            "Content-Length: %d\r\n"
                            "\r\n",
                 content_length);
        write(connfd, buff, strlen(buff));

        while (true)
        {
            char buff[512];
            ssize_t ret = read(page_fd, buff, 512);
            if (ret != 0)
                write(connfd, buff, ret);
            else
                break;
        }
    }
    else
    {
        char path[512];
        snprintf(path, 512, "static%s", uri);
        if (access(path, R_OK) == 0)
        {
            printf("File Access OK \n");
        }
        else
        {
            printf("Invalid File Access\n");
            int page_fd = open("static/404.html", O_RDONLY);
            struct stat st;
            fstat(page_fd, &st);
            size_t content_length = st.st_size;
            char buff[512];
            snprintf(buff, 512, "HTTP/1.1 404 OK \r\n"
                                "Content-Type: text/html \r\n"
                                "Content-Length: %d\r\n"
                                "\r\n",
                     content_length);
            write(connfd, buff, strlen(buff));
            while (true)
            {
                char buff[512];
                ssize_t ret = read(page_fd, buff, 512);
                if (ret != 0)
                    write(connfd, buff, ret);
                else
                    break;
            }
        }
    }

    close(connfd);
    sem_post(&num_of_active_thread);

    printf("==================== Request Ended ====================== \n");
}

int main(int argc, char **argv)
{
    sem_init(&num_of_active_thread, false, MAXTHREAD);

    int listenfd, connfd;

    struct sockaddr_in servaddr, cliaddr;
    socklen_t len = sizeof(struct sockaddr_in);

    char ip_str[INET_ADDRSTRLEN] = {0};

    int threads_count = 0;
    pthread_t threads[MAXTHREAD];

    /* initialize server socket */
    listenfd = socket(AF_INET, SOCK_STREAM, 0); /* SOCK_STREAM : TCP */
    if (listenfd < 0)
    {
        printf("Error: init socket\n");
        return 0;
    }

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
        connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &len);
        if (connfd < 0)
        {
            printf("Error: accept\n");
            return 0;
        }

        /* print client (remote side) address (IP : port) */
        inet_ntop(AF_INET, &(cliaddr.sin_addr), ip_str, INET_ADDRSTRLEN);
        printf("Incoming connection from %s : %hu with fd: %d\n", ip_str, ntohs(cliaddr.sin_port), connfd);

        /* create dedicate thread to process the request */
        pthread_t tid;
        if (pthread_create(&tid, NULL, request_func, &connfd) != 0)
        {
            printf("Error when creating thread %d\n", threads_count);
            return 0;
        }
        pthread_detach(tid);
    }

    return 0;
}