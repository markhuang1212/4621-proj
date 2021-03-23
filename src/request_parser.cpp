#include <string>
#include <streambuf>
#include <optional>
#include "src/http_header_object.cpp"

#ifndef request_parser_h
#define request_parser_h

class request_parser
{
    bool is_header_complete = false;

    std::optional<http_header_object> header;
    std::string buffer = "";

    void write(std::string str)
    {
        buffer.append(str);

        int N = buffer.size();
        for (int i = 0; i < N - 4; i++)
        {
            if (str.substr(i, 4) == "\r\n\r\n")
            {
                is_header_complete = true;
                parse_header(buffer.substr(0, i + 4));
                buffer.erase(buffer.begin(), buffer.begin() + i + 4);
            }
        }
    }

    void parse_header(std::string str)
    {
        std::string method;
        std::string path;
        for (int i = 0; true; i++)
        {
            if (str[i] == ' ')
            {
                if (method == "")
                {
                    method = str.substr(0, i);
                }
                else
                {
                    path = str.substr(method.size() + 1, i);
                    break;
                }
            }
        }

        http_header_object header;
        header.method = method;
        header.url = path;
        this->header = header;
    }

    std::optional<http_header_object> read()
    {
        return header;
    }
};

#endif