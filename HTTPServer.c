#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <stddef.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <stdbool.h>

//The port number is hard coded here mostly for the easy of use and testing.
#define DEFAULTPORT 8889

//This variable is used to keep the main loop running.
static bool keepRunning = true;

//Changes the keep running variable to 0 to stop the main loop.
void intHandler(int val){
	keepRunning = false;
}

int main(int argc, char const *argv[]) {
	
	//Signal catch to catch ctrl-c and start the shutdown process.
	struct sigaction act;
	act.sa_handler = intHandler;
	sigaction(SIGINT, &act, NULL);
	
	//Variable to hold on to passed or default port number
	int port;
	
	if(argc < 2){
		printf("No port number supplied, using default port %d\n", DEFAULTPORT);
		port = DEFAULTPORT;
	}
	else{
		port = atoi(argv[1]);
	}
	
	
    //PID of child processes
    pid_t childpid;

    //This will eventually hold the file descriptor of the server socket.
    int serverSocket;

    //This will eventually hold the file descriptor of the new socket that is created when a client connects.
    int newSocket;

    //This will eventually be used by read to hold an incoming message.
    long valread;

    //The socket address structure that is pre defined. This will be used later after we set some variables to
    //call bind.
    struct sockaddr_in address;

    //Get the size of address and store it for easier use later.
    int addrlen = sizeof(address);

    //Variable that will eventually hold the .gif file that is sent to the client.
    FILE *fp;

    //Open the .gif file in read in the bytes.
    fp = fopen("comic.gif", "rb");
	
	if(fp == NULL){
		perror("Failed to open comic.gif");
		return 1;
	}

    //Stat struct to get info about the .gif file.
    struct stat st;

    //Here we associate the stat st struct with the comic.gif file.
    stat("comic.gif", &st);

    //Store the size of the comic file.
    long int size = st.st_size;

    //Buffer for the gif file that needs to be sent.
    char file_buff[size+1];

    /*
     * size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
     * fread is used to read in the .gif file to the file_buff, the second argument is the size of an element to read,
     * the third argument is the number of elements to read, and the fourth argument is the file we are reading from.
     */
    fread(&file_buff, sizeof(int), st.st_size+1, fp);

    //Close the file pointer after we have read it in.
    fclose(fp);

    //A char array that will be used to hold the content length when its converted from long int to string.
    char tmp[255];

    //Convert size from a long int to a string and store it in tmp.
    sprintf(tmp, "%ld", size);

    //Strings that are used to build the various parts of the response message..
    char *response = "HTTP/1.1 200 OK\n";
    char *contentType = "Content-Type: image/gif\n";
    char *contentLengthHeader = "Content-length: ";
    char *connectionHeader = "\nConnection: close\n\n";

    //Calculate the header buffer size needed for the response header.
    int headerBuffSize = sizeof(response)+ sizeof(contentType) + sizeof(contentLengthHeader) + sizeof(connectionHeader) +
            sizeof(tmp) + st.st_size;

    //Buffer that will be used to contain the combined header response.
    char str[headerBuffSize];

    //String concatenation of the header response parts.
    strcpy(str, response);
    strcat(str, contentType);
    strcat(str, contentLengthHeader);
    strcat(str, tmp);
    strcat(str, connectionHeader);
    memcpy(str+strlen(str), file_buff, st.st_size);


    /*Tries to set the server socket. Since the socket is called with socket(domain, type, protocol) we set it as such
     * domain   = AF_INET, this setting the communication domain to an IP address type.
     * type     = SOCK_STREAM, this is ensuring that our connection type is a TCP connection instead of a UDP connection.
     * protocol = 0, since there are no variations of the protocol.
     * If the socket cannot be created we print out an error message and then exit.
    */
    if((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        perror("An Error occurred when trying to create the socket.");
        exit(EXIT_FAILURE);
    }

    /*
     * Address is just a generic container there are some variables that need to be assigned before it can be used
     * to bind a socket.
     * address.sin_family = AF_INET, this is the same as when we first set up the socket.
     * sin_port           = PORT, this is port number we will use to for clients to connect to.
     * sin_addr.s_addr    = 0.0.0.0, This is the address for the socket, we are setting it to 0.0.0.0 so that we can
     *                      let the operating system choose the address
     */
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;

    //htons is used to convert a short integer to a network representation.
    address.sin_port = htons(port);

    /*
     * memset() is used to fill a block of memory with a particular value. It has the following syntax
     * void *memset(void *ptr, int x, size_t n)
     * ptr = starting address of memory to be filled.
     * x   = The value to be field.
     * n   = The number of bytes to be filled starting from ptr to be filed.
     */
    memset(address.sin_zero, '\0', sizeof(address.sin_zero));

    /*
     * The bind system call is used to assign, or bind, a port number to a socket. The transport address, port number,
     * is defined in the address struct. bind has the following function structure:
     * int bind(int socket, const struct sockaddr *address, socklen_t address_len);
     * socket    = This is the socket that is created above.
     * sockaddr  = This is the generic structure that address is used above, it
     */
    if(bind(serverSocket, (struct sockaddr *)&address, sizeof(address)) < 0){
        perror("An error occurred while trying to bind the server socket.");
        exit(EXIT_FAILURE);
    }

    /*
     * The listen system call indicates that we want to listen for incoming connections from clients.
     * It has the following function call format: listen(int socket, int backlog)
     * socket  = serverSocket, this is the servers main socket.
     * backlog = This is the number of maximum pending connections out server can have before it starts to refuse
     *           connections.
     */
    if(listen(serverSocket, 10) < 0){
        perror("An error occurred while listening for incoming connections");
        exit(EXIT_FAILURE);
    }

    while(keepRunning){
        printf("\n++++++++++++++ Waiting for new connection ++++++++++++++++ \n\n");

        /*
         * Accept is used to grab a connection request off of a queue of pending requests. Here we set
         * newSocket to an incoming request. Accept has the following function call
         * int accept(int socket, struct sockaddr *restrict address, socklen_t *restrict address_len)
         * socket      = this is the same socket that is used in listen, the serverSocket.
         * address     = This struct gets filled in when a client connects with their address and port number so we
         *               later examine it if need be.
         * address_len = this is the length of the address struct.
         */
        if((newSocket = accept(serverSocket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0){
            perror("\nAn error occurred while trying to accept an incoming connection.");
            exit(EXIT_FAILURE);
        }

        if((childpid = fork()) == 0){

            //Close the server socket file descriptor in the child process.
            close(serverSocket);

            while(1){
                /*
                 * buffer array for 30,000 bytes with the first element set to zero. Used for reading incoming messages.
                 */
                char buffer[30000] = {0};

                /*
                 * ssize_t read(int fd, void *buf, size_t count)
                 * read is used to read an incoming message from the newSocket file descriptor.
                 * fd   = newSocket, our client servicing socket file descriptor.
                 * *buf = buffer, start reading at buffer location.
                 * count = number bytes to read up to.
                 */
                valread = read(newSocket, buffer, 30000);
                printf("Received: \n");
                printf("%s", buffer);
                //Since each line in the http header ends with \r\n I set find to \r
                char find = '\r';

                //Here I find the request header and store it
                const char *lineEnd = strchr(buffer, find);
                if(lineEnd) {
                    //Calculate where the line ending index is.
                    int index = lineEnd - buffer;

                    //Create an char array to hold the request header.
                    char requestLine[index+1];

                    //Copy the request header from the buffer.
                    memcpy(requestLine, buffer, index);

                    //Terminate the request header string.
                    requestLine[index] = '\0';

                    //Duplicate the request line to parse it.
                    char * requestLineDup = strdup(requestLine);

                    //Holds the incoming method type.
                    char * method;

                    //Holds the incoming request uri.
                    char * requestURI;

                    //Store the space character to be used as a delimiter.
                    const char spaceDelim[] = " ";

                    //Separate the method and request URI from the request line.
                    method     = strsep(&requestLineDup, spaceDelim);
                    requestURI = strsep(&requestLineDup, spaceDelim);

                    //If the client is asking for a GET request.
                    if((strcmp(method, "GET")) == 0){

                        //Verify the client is asking for the comic gif.
                        if((strcmp(requestURI, "/comic.gif")) == 0){
                            /*
                             * ssize_t write(int fd, const void *buf, size_t count);
                             * write() writes up to count bytes from the buffered pointed buf to the file referred to by the file
                             * descriptor  fd.
                             * fd   = newSocket, the socket we want to write to.
                             * *buf = hello, this is our message we want to write in to respond to a client.
                             * count = strlen(hello), take the length of hello and set it to how many bytes we want to write out.
                             */
                            write(newSocket, str, strlen(str) + st.st_size);

                            //Clear out the buffer
                            bzero(buffer, sizeof(buffer));
                        }
                        //If the client is requesting get of a different file.
                        else{
                            char * invalidGet = "HTTP/1.1 404 FILE NOT FOUND\nContent-Type: text/plain\nContent-Length: 23\n\nThat file doesnt exist.";
                            write(newSocket, invalidGet, strlen(invalidGet));
                        }
                        pid_t a = getpid();
                        kill(a, SIGKILL);
                        close(newSocket);
                    }
                }
            }
        }
    }

    /*
     * close() closes a file descriptor, so that it no longer refers to any file and may be reused.
	 * close both the newSocket and the serverSocket.
     */
    close(newSocket);
	close(serverSocket);
	
	//Free up the serverSocket so it can be reused without error.
	int isTrue = 1;
	setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &isTrue, sizeof(int)); 

    return 0;
}

