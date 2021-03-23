#include <boost/asio.hpp>
#include <functional>
#include <src/HttpRequest.cpp>
#include <src/HttpResponse.cpp>
#include <sys/socket.h>
#include <sys/types.h>

#ifndef http_server_h
#define http_server_h

void EmptyHandler(HttpRequest &req, HttpResponse &res)
{
    return;
};

class HttpServer
{
    boost::asio::thread_pool &tp;
    std::function<void(HttpRequest &, HttpResponse &)> cb;
    std::thread main_execution;

    int serverFd = -1;

    void __receiveConnections() // called in main_execution
    {
        while (true)
        {
            sockaddr_in addr;
            int connectFd = connect(serverFd, (sockaddr *)&addr, sizeof(struct sockaddr_in));
        }
    }

public:
    HttpServer(boost::asio::thread_pool &tp)
        : tp(tp), cb(EmptyHandler)
    {
    }

    void listen(int port)
    {
    }

    void stop()
    {
        
    }

    void setHandler(std::function<void(HttpRequest &, HttpResponse &)> cb)
    {
        this->cb = cb;
    }
};

#endif