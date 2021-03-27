#include "src/HttpServer.cpp"
#include "src/StaticFileHandler.cpp"
#include "unistd.h"

using namespace std;

int main()
{
    // Create a HTTP server, which handles static files from the directory /static
    // see src/HttpServer.cpp src/StaticFileHandler.cpp
    HttpServer server(StaticFileHandler("static"));

    // Listen at port 8080
    server.listen(8080);
    
    // Pause the main thread
    pause();
    
    return 0;
}