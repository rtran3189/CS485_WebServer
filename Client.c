/* Client-side program that is used to interact with the HTTP Web Server.
* @author Richard Tran, Eric Turnbull
*/
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

char serverAddress[100] = "";
short serverPort = 8080;
char fileName[100] = "";
/*
* This function simply takes in an input and the length and 
* creates a string from the input in order to be parsed 
* to create the connection.
*/
char *readLine(char * input, int length) 
{
    int i;
    printf("Input URL: ");
    for(i = 0; i < length; i++)
    {
        input[i] = fgetc(stdin);
        input[length] = '\0';
        if(input[i] == '\n')
            break;
   }
   return input;
}

/*
* This function takes the user input and parses it using strtok and tries to
* create a connection from it.
* Proper format will be: http://<ip>:<port>/<else>
*   example: http://127.0.0.1:8080/comic.gif
*/
void parseInput(char* input)
{
    char *newTest = (char*)malloc(sizeof(char)*100);
    char *cpyInput = (char*)malloc(sizeof(char)*100);
    char *unparsedAdd = (char*)malloc(sizeof(char)*100);
    char delim[] = "/";

    // Save the original string 
    memcpy(cpyInput, input, sizeof(char)*100);
    memcpy(newTest, input, sizeof(char)*100);

    // Expected to be the http: part of the input
    char *splitInput = strtok(newTest, delim);
    if(!!strcmp(splitInput, "http:") && !!strcmp(splitInput, "HTTP:"))
    {
        printf("ERROR: Invalid format, please include HTTP.\n");
        exit(1);
    }

    // Expected to be the IP and port part of the input
    splitInput = strtok(NULL, delim);
    memcpy(unparsedAdd, splitInput, 100);

    // Split the address and port to parse. Gets the server address.
    splitInput = strtok(unparsedAdd, ":");
    memcpy(serverAddress, splitInput,1000);

    // Get the server port.
    splitInput = strtok(NULL, "\0");
    serverPort = atoi(splitInput);

    // Parse the file name.
    splitInput = strtok(cpyInput, delim);
    splitInput = strtok(NULL, delim);
    splitInput = strtok(NULL, "\n");
    memcpy(fileName, splitInput, 1000);

    printf("==== INFO ====\n");
    printf("Server address: %s\n", serverAddress);
    printf("Port number: %d\n", serverPort);
    printf("File name: %s\n", fileName);
}

/*
* This function establishes the connection to the server
* using the parsed information. It also echos the reply from
* the server.
*/
void createConnection()
{
    char buffer[30000] = {0};
    int BUFFER_SIZE = 1000;
    char inputRequest[BUFFER_SIZE+1];
    int sock = 0; long valread;

    // This is a structure used to handle IP sockets.
    // refer to man pages for details with 'man ip'.
    struct sockaddr_in serv_addr;
    // The request itself.
    char *request = "GET /comic.gif HTTP/1.1\r\n";
    char *requestMethod = "GET /";
    char *reqTest = "comic.gif";
    char *requestType = " HTTP/1.1\r\n";
    char requestArr[1000] = "";

    // If for some reason the socket can't be created, give output and
    // exit the program.
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        exit(1);
    }
    
    memset(&serv_addr, '0', sizeof(serv_addr));
    
    // sin_family is the address family of the socket address.
    serv_addr.sin_family = AF_INET;
    // sin_port is the port in network byte order.
    serv_addr.sin_port = htons(serverPort);
    
    // Checks to see if the addresses are of a valid form.
    if(inet_pton(AF_INET, serverAddress, &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address. \n");
        exit(1);
    }
    
    // If connecting to the server fails for some reason, output
    // failure message and exit the program.
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        exit(1);
    }

    printf("Connection established!\n");
    strcpy(requestArr, requestMethod);
    strcat(requestArr, fileName);
    strcat(requestArr, requestType);
    printf("Sending request: %s\n", requestArr);
    // Send the actual request.
    send(sock , requestArr , strlen(request) , 0 );
    printf("Request sent.\n");
    printf("\nServer response: \n");
    // Read the message received from the server and print it.
    valread = read( sock , buffer, 30000);
    printf("%s\n",buffer );
}

/*
* Client main.
*/
int main(int argc, char *argv[])
{
    // Checks if the optional command line argument was supplied at run time.
    if (argc >= 2)
    {
        parseInput(argv[1]);
    }
    else
    {
        int BUFFER_SIZE = 1000;
        char input[BUFFER_SIZE+1];
        parseInput(readLine(input, BUFFER_SIZE));
    }

    createConnection();
}