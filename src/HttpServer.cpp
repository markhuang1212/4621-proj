#include <queue>
#include <functional>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <optional>
#include <stdexcept>

#include "src/ThreadPool.cpp"
#include "src/HttpRequest.cpp"
#include "src/HttpResponse.cpp"

#include "semaphore.h"
#include "pthread.h"
#include <sys/types.h>
#include <sys/socket.h>
#include "netinet/in.h"
#include "arpa/inet.h"
#include "string.h"
#include <cstdio>

#ifndef http_server_h
#define http_server_h

using namespace std;

auto c_listen = listen;

class HttpServer
{
private:
    int serverFd;
    optional<ThreadPool> tp;
    atomic<bool> isListening;
    function<void(HttpRequest, HttpResponse)> handler;

    void handleConnection(int connFd)
    {
    }

    void acceptConnections()
    {
        while (true)
        {
            sockaddr_in addr;
            int connFd;
            int len = sizeof(sockaddr_in);

            connFd = accept(serverFd, (sockaddr *)&addr, (socklen_t *)&len);
            if (connFd < 0)
            {
                printf("Error: Accept Socket Connection Failed");
                continue;
            }

            char ip_str[INET_ADDRSTRLEN] = {0};
            inet_ntop(AF_INET, &(addr.sin_addr), ip_str, INET_ADDRSTRLEN);
            printf("Incomming Connection From %s:%hu", ip_str, ntohs(addr.sin_port));

            tp.value().addTask(bind(&HttpServer::handleConnection, this, connFd));
            this_thread::yield();
        }
    }

public:
    HttpServer(const function<void(HttpRequest, HttpResponse)> &handler)
        : handler(handler) {}

    void listen(int port)
    {
        if (isListening)
        {
            throw runtime_error("HTTP Server Already Listening");
        }
        if (!tp.has_value())
        {
            tp.emplace(256);
        }

        sockaddr_in addr;
        socklen_t len = sizeof(sockaddr_in);

        serverFd = socket(AF_INET, SOCK_STREAM, 0);
        if (serverFd < 0)
        {
            throw runtime_error("Error creating server socket");
        }

        memset(&addr, 0, len);
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);

        if (bind(serverFd, (sockaddr *)&addr, len) < 0)
        {
            throw runtime_error("Error binding socket to port");
        }

        if (c_listen(serverFd, 64) < 0)
        {
            throw runtime_error("Error listening to port");
        }

        tp.value().addTask(bind(&HttpServer::acceptConnections, this));
    }
};

#endif