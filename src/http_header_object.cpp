#include <cstring>
#include <string>

#ifndef http_header_object_h
#define http_header_object_h

class http_header_object
{
public:
    std::string method = "GET";
    std::string url = "";
};

#endif