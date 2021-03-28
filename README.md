# COMP4621 Project Assignment

> A static web server written in C++

## Build and Run

To build the server, you need

* g++ that supports c++14
* zlib

Note: the cse LAB machine's default g++ version DOES NOT support c++14. It does have `zlib` preinstalled.


To build the project, just run `make`. The outputs (including all tests) are at `/out`

To start the server, run

```bash
./main "/the/dir/to/serve"
```

## Features

Serve static files with

1. Appropriate MIME type
2. support for pause-and-continue downloading.
3. Compress files with `gzip` format when transferring

## Implementation

The code is fully odject oriented and self-explainatory. With the `main.cpp` as simple as

```cpp
int main(int argc, char** argv){
    
    checkArguments(argc, argv);

    HttpServer server(StaticFileHandlers(argv[1]));

    server.listen(PORT);

    return 0;

}
```

Several classes handles jobs exactly as their name indicates. Detailed descriptions are in the source code's comment.

```cpp
class ThreadPool;
class HttpServer;
class HttpRequest;
class HttpResponse;
class StaticFileHandler;
class MimeTypes;
class ZlipWrapper;
```