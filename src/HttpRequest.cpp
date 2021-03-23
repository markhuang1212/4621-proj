#include <string>
#include <sstream>
#include <atomic>

#ifndef http_request_h
#define http_request_h

class HttpRequest
{
private:
    std::atomic<int> fd;
    std::atomic<bool> ready;
    std::stringstream raw_request;

public:
    HttpRequest(int fd) : fd(fd)
    {
    }
};

#endif