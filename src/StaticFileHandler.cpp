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

    void operator()(HttpRequest &req, HttpResponse &res)
    {
        if (req.getUrl() == "/")
        {
            res.sendFile("index.html");
            return;
        }
        else
        {
            res.sendFile(req.getUrl().substr(1));
            return;
        }
    }
};

#endif