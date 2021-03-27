#include "string"
#include "src/HttpRequest.cpp"
#include "src/HttpResponse.cpp"

#ifndef static_file_handler
#define static_file_handler

using namespace std;

class StaticFileHandler
{

public:
    StaticFileHandler(string path)
    {
    }

    void operator()(const HttpRequest &req, const HttpResponse &res)
    {
    }
};

#endif