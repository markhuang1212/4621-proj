# HTTP Server

The source code is at `src/main.c`.

## Features

* It runs as a static file server, serving files under the directory `/static`. 
* Url `/` is directed to `/index.html`. 
* It determines the MIME type of a file by its extension.
* When a resource is not found, it returns `/static/404.html` with a status code 404.

## Additional Features

* The browser caches the content by the `Cache-Control` header.
* The server send its content using `deflate` compression algorithm, which all browser nowadays support. (By using the `zlib.h` library)

## Build

The program can build and run on the cs lab machine.

```bash
# Build the binary
# Use the newest version of gcc available on the lab machine
echo "CC=gcc10 make" | bash
# Run the binary
src/main
```

## Implementation

Implemented in c. 

I first copy the source code from Lab3, then made the following major change:

* Feature: the main logic of HTTP is implemented in `void* request_func(void*)`
* Error handling: non-fatal error will not cause the server to terminate
* Threading: Use a semaphore to count running thread. Server can run forever
* Threading: A thread either exits voluntarily, or is automatically killed after 30 seconds

## Author

Huang Meng (mhuangaj@connect.ust.hk)