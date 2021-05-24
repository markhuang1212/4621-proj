# HTTP Server

> This project was a course assignment of COMP4621(Networking) at HKUST.

The source code is at `src/main.c`.

## Features

* The program runs as a static file server, serving all files under the directory `./static`. To let the server runs properly, `index.html` and `404.html` is required.
* Url `/` is directed to `/index.html`. 
* It determines the MIME type of a file by its extension, so that the browser can process them correctly
* When a resource is not found, it returns `/static/404.html` with status code `404`.

## Additional Features

* The browser caches the content via the `Cache-Control` header.
    * Motivation: browser doesn't need to load a resource that it has loaded twice
* The server send its content using `deflate` compression algorithm, which all browser nowadays support. (By using the `zlib.h` library)
    * Motivation: The transmission size is much smaller after compression, so the loading time will decrease, especially under not-so-well network condition.

## Build

The program can be built and run on the cse lab machine, or any Linux machine with `glibc`, `gcc(>=9)` and `zlib`. It is NOT Posix compatible, as it uses some linux only system calls, e.g. `accept4()`, `pthread_yield()`

```bash
# Build the binary
# Use the newest version of gcc available on the cse lab machine
echo "CC=gcc10 make" | bash
# Run the binary
# Port can be specified by the `SERVER_PORT` environment variable
echo "SERVER_PORT=8081 src/main" | bash
```

## Implementation

The program is implemented in c. 

I first copy the source code from Lab3, then made the following major change:

* Feature: the main logic of HTTP is implemented in `void* request_func(void*)`
* Feature: the compressing part is implemented in `int pipe_fd(int src, int dest)`
* Feature: the MIME determination is implemented in `int ext_to_mime(char *buff, size_t buff_len)`
    * More types can be easily supported by editing that function
* Socket: the connection socket is non-blocking, so that thread can gracefully exit after timeout
* Error handling: non-fatal error will not cause the server to terminate, only the corresponding TCP connection will terminate
* Threading: Use a semaphore to count running thread. Server can run forever without "running out of threads"
* Threading: A thread terminates itself if the client is unresponsive for 30 seconds.
* Security: HTTP Requests like `GET /../../../../../etc/passwd HTTP/1.1` will be rejected with 404.

## Caevats

* The url in the HTTP request does not accept the `%XX` hex format. E.g. `%2E` will not be converted to the space character.

## Author

Huang Meng (mhuangaj@connect.ust.hk)

May 16, 2021
