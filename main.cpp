#include <string>
#include <stdexcept>

#include "sys/socket.h"
#include "netinet/in.h"
#include "arpa/inet.h"

int create_server_socket(int port)
{
    int capacity = 100;

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    int socketfd = socket(AF_INET, SOCK_STREAM, 0);

    if (socketfd < 0)
    {
        throw std::runtime_error("Cannot create server socket.");
    }

    if (bind(socketfd, (sockaddr *)&addr, sizeof(addr)) < 0)
    {
        throw std::runtime_error("Cannot listen to address.");
    }

    if (listen(socketfd, capacity) < 0)
    {
        throw std::runtime_error("Cannot listen to address.");
    }

    return socketfd;
}

int main()
{
}