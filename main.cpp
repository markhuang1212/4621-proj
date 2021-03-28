#include "src/HttpServer.cpp"
#include "src/StaticFileHandler.cpp"
#include "unistd.h"


using namespace std;

int main(int argc, char **argv)
{

    HttpServer server(StaticFileHandler("static"));
    server.listen(8080);
    pause();

    return 0;
}