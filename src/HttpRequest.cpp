#ifndef http_request_h
#define http_request_h

#include "stdio.h"
#include "unistd.h"
#include "string.h"
#include <string>
#include <map>
#include <variant>
#include <vector>

using namespace std;

class HttpRequest
{

    FILE *file;

    string method;
    string url;

public:
    HttpRequest(FILE *file)
        : file(file)
    {
        // parse the first line
        char *buff = NULL;
        size_t len = 0;
        int ret = getline(&buff, &len, file);

        char *pch = strtok(buff, " \r\n");
        method = pch;
        pch = strtok(NULL, " \r\n");
        url = pch;

        free(buff);
        buff = NULL;
        len = 0;
        ret = getline(&buff, &len, file);
        while (ret != 2)
        {
            free(buff);
            len = 0;
            ret = getline(&buff, &len, file);
        }
    }

    string getMethod()
    {
        return method;
    }
    string getUrl()
    {
        return url;
    }
};

#endif