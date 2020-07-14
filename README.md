# Lab 2: HTTP Client/Server Model

### Created by: Richard Tran and Eric Turnbull

This is a multi-threaded C program that runs a server for hosting the required comic.gif file for clients to view.

# How to compile

To compile this program, simply `cd` into the `lab2` directory and run the command: `make all`.

# How to run

First before you run the program, make sure it is compiled. The server needs to run first before clients can connect to it. See below for instructions on how to start the server.

## Starting the server

To run the server, make sure it is first compiled and then, using a command line, launch the server with `./webserver [port number]`. for example say you wish to use port 8854 you would use `./webserver 8854`. You can also use `./webserver` and the server will default to port 8889. After successful server set up, you should see that the server is waiting for connections.

## Creating and connecting clients to the server

To start connecting clients to the server, make sure the server is running successfully and is awaiting connections. Make sure the program is compiled and then run `./client [url]`, For example, `./client https://localhost:8889/comic.gif`. You can also run simply `./client` and the program will prompt for a URL address. Upon successful connection the terminal should print the response header along with the the requested information. After the request has been responded to, the current connection will be closed to allow for other clients to connect. If you try to use a method other then get you will be greeted with a 404 and if you try to get a different file you will be greeted with a message indicating that file does not exist on this server.

# Shutting the Server Down.

When you are finished with the server you can press CTRL+C to shut the server down and the used port number should be freed.

# Sample runs
This is an example of a successful request without supplying command line arguments.
```richardtran@rs-lnx-098:~/Documents/Class/lab2$ ./client
Input URL: http://127.0.0.1:8889/comic.gif
==== INFO ====
Server address: 127.0.0.1
Port number: 8889
File name: comic.gif
Connection established!
Sending request: GET /comic.gif HTTP/1.1

Request sent.

Server response: 
HTTP/1.1 200 OK
Content-Type: image/gif
Content-length: 20189
Connection: close

GIF87aX��
```


This is an example of an invalid request without command line arguments.
```
richardtran@rs-lnx-098:~/Documents/Class/lab2$ ./client
Input URL: http://127.0.0.1:8889/comic2.gif
==== INFO ====
Server address: 127.0.0.1
Port number: 8889
File name: comic2.gif
Connection established!
Sending request: GET /comic2.gif HTTP/1.1

Request sent.

Server response: 
HTTP/1.1 404 FILE NOT FOUND
Content-Type: text/plain
Content-Length: 23

That file doesn't exist.
```

This is an example of a successful request when supplying command line arguments.
```
richardtran@rs-lnx-098:~/Documents/Class/lab2$ ./client http://127.0.0.1:8889/comic.gif
==== INFO ====
Server address: 127.0.0.1
Port number: 8889
File name: comic.gif
Connection established!
Sending request: GET /comic.gif HTTP/1.1

Request sent.

Server response: 
HTTP/1.1 200 OK
Content-Type: image/gif
Content-length: 20189
Connection: close

GIF87aX��
```

This is an example of an invalid request with command line arguments.
```
richardtran@rs-lnx-098:~/Documents/Class/lab2$ ./client http://127.0.0.1:8889/comic2.gif
==== INFO ====
Server address: 127.0.0.1
Port number: 8889
File name: comic2.gif
Connection established!
Sending request: GET /comic2.gif HTTP/1.1

Request sent.

Server response: 
HTTP/1.1 404 FILE NOT FOUND
Content-Type: text/plain
Content-Length: 23

That file doesnt exist.
```