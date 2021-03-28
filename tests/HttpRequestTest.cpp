#include <bits/stdc++.h>
#include "src/HttpRequest.cpp"

using namespace std;

int main()
{
    int fd[2];
    pipe(fd);
    FILE *file_w = fdopen(fd[1], "w");
    FILE *file_r = fdopen(fd[0], "r");

    fprintf(file_w, "GET /index.html\r\n");
    fprintf(file_w, "User-Agent: chrome\r\n");
    fprintf(file_w, "\r\n");
    fflush(file_w);

    HttpRequest req(file_r);

    cout << req.getMethod() << endl;
    cout << req.getUrl() << endl;

    return 0;
}