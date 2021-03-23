#include <string>
#include <streambuf>
#include <optional>
#include <algorithm>
#include "src/http_header_object.cpp"

#ifndef request_parser_h
#define request_parser_h

class request_parser
{

private:
    bool is_header_complete = false;

    std::optional<http_request_header> header;
    std::string buffer = "";

public:
    void write(std::string str)
    {
        buffer.append(str);

        int N = buffer.size();
        for (int i = 0; i < N - 3; i++)
        {
            if (buffer.substr(i, 4) == "\r\n\r\n")
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
                method = str.substr(0, i);
                path = str.substr(i + 1);
                for (int i = 0; path.size(); i++)
                {
                    if (path[i] == ' ' || path[i] == '\r')
                    {
                        path.erase(path.begin() + i, path.end());
                        break;
                    }
                }
                break;
            }
        }

        http_request_header header;
        header.method = method;
        header.url = path;
        this->header = header;
    }

    std::optional<http_request_header> getHeader()
    {
        return header;
    }

    std::optional<std::string> read(std::string encoding = "ascii")
    {
    }
};

#endif