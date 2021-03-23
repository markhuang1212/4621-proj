#include <cstring>
#include <string>
#include <vector>
#include <map>

#ifndef http_header_object_h
#define http_header_object_h

class http_header_object
{
private:
public:
};

class http_request_header : public http_header_object
{
public:
    std::string method = "";
    std::string url = "";
    std::vector<std::string> accept_encoding = {"gzip"};
};

class http_response_header : public http_header_object
{
    
};

#endif