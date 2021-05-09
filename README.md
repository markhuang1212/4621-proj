# HTTP Server

The source code is at `src/main.c`.

## Features

* It runs as a static file server, serving files under the directory `/static`. 
* Url `/` is directed to `/index.html`. 
* It determines the MIME type of a file by its extension.
* When a resource is not found, it returns `404`.

## Additional Features

* The browser caches the content.
* Cookie is set to greet the visitor. (Redis is used to store data)

## Implementation

Implemented in c. 

I first copy the source code from Lab3, then made the following major change:

* Feature: the main logic of HTTP is implemented in `void* request_func(void*)`
* Error handling: non-fatal error will not cause the server to terminate
* Threading: Use a semaphore to count running thread. Server can run forever
* Threading: A thread either exits voluntarily, or is automatically killed after 30 seconds