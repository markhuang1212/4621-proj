#ifndef http_response_h
#define http_response_h

#include "unistd.h"
#include "stdio.h"
#include <string>

using namespace std;

class HttpResponse
{

    FILE *file;

public:
    HttpResponse(FILE *file)
        : file(file)
    {
    }

    void sendFile(string path)
    {
    }
};

#endif