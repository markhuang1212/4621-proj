#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <time.h>
#include <pthread.h>

#include <semaphore.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_PORT (8080)
#define LISTENNQ (8)
#define MAXLINE (100)
#define MAXTHREAD (8)
#define true (1)
#define false (0)

sem_t num_of_active_thread;

void *request_func(void *args)
{
    /* get the connection fd */
    int connfd = *(int *)args;
    char buff[MAXLINE] = {0};

    FILE* connf = fdopen(connfd, "r+");
    fprintf(connf, "This is the content sent to connection %d\r\n", connfd);

    close(connfd);
    sem_post(&num_of_active_thread);
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