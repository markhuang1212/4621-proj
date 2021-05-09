# HTTP Server

The source code is at `src/main.c`.

## Features

* It runs as a static file server, serving files under the directory `/static`. 
* Url `/` is directed to `/index.html`. 
* It determines the MIME type of a file by its extension.
* When a resource is not found, it returns `404`.