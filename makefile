all: webserver client

clean:
	rm webserver client

webserver: HTTPServer.c
	gcc -o webserver HTTPServer.c

client: Client.c
	gcc -o client Client.c