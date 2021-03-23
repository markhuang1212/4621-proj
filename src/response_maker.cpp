#include <optional>
#include <string>
#include "src/http_header_object.cpp"

#ifndef response_maker_h
#define response_maker_h

class response_maker
{

    std::string response;

public:
    void write(http_request_header header)
    {
        auto method = header.method;
        auto path = header.url;

        if (method != "GET")
        {
            response = "";
        }
    }

    std::optional<std::string> read()
    {
    }
};

#endif