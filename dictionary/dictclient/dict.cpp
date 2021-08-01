/** This program is the command line client program for dictionary management.
  * It is invoked with: 
  *     "dict <insert|delete|search> <word>"
  * where <word> is an English-language word.
  * Also supported are:
  *     "dict <save|quit>"
  * where quit causes the server to exit and save saves the dictionary to a predefined file.
  **/
#include <cstdio>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
 
using namespace std;

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 27027    // this is the dictionary server port

typedef int SOCKET;

extern void usage(char* progname);

int main(int argc, char* argv[]) {
     char *reqbuf;
     int reqlen;
     // First validate the command line
     if (argc < 2 || argc > 3) {
         usage(argv[0]);
         return -1;
     }
     if (strcmp(argv[1], "insert") == 0 ||
         strcmp(argv[1], "delete") == 0 ||
         strcmp(argv[1], "search") == 0) {
             if (argc != 3) {
                 usage(argv[0]);
                 return -1;
             }
        reqlen = strlen(argv[1]) + strlen(argv[2]) + 2;
        reqbuf = new char[reqlen];
        sprintf(reqbuf, "%s %s", argv[1], argv[2]);
     } else if (strcmp(argv[1], "save") == 0 ||
                strcmp(argv[1], "quit") == 0) {
                    if (argc != 2) {
                        usage(argv[0]);
                    }
        reqlen = strlen(argv[1]) + 1;
        reqbuf = new char[reqlen];
        sprintf(reqbuf, argv[1]); 
     }
     // request has been validated; now initialize communications
    SOCKET connectSocket;
    connectSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (connectSocket < 0) {
    	perror("Error opening socket");
    	return connectSocket;
    }
    struct sockaddr_in serv_addr;
    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(DEFAULT_PORT);

    int n;
    if ((n = connect(connectSocket, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
    	perror("Error connecting");
    	return n;
    }
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    // Send the request to the server
    n = send(connectSocket, reqbuf, reqlen, 0 );
    printf("Sent %d bytes to server: %s\n", n, reqbuf);
    if (n < 0) {
    	perror("Error writing to socket");
    	close(connectSocket);
        return n;
    }

    // now wait for the response
        // Receive until the peer closes the connection
    int rval = 1;
    do {

        n = recv(connectSocket, recvbuf, recvbuflen, 0);
        if ( n > 0 ) {
            recvbuf[n] = '\0'; // we know that the response will be shorter than recvbuf length
            printf(recvbuf);
            if (strcmp(recvbuf,"failed") == 0) {
                rval = -1;
                printf("\nRequest failed\n");
            }
            else if (strcmp(recvbuf,"success") == 0) {
                rval = 0;
                printf("\nRequest succeeded\n");
            }
        }
        else if ( n == 0 ) {
            printf("Connection closed\n");
            if (rval == 1)
                rval = -1;      // closed without getting an answer
        }
        else {
            perror("Error on recv");
            rval = -1;
        }
    } while( n > 0 );

    // cleanup
    close(connectSocket);
    return rval;
}

void usage(char* progname) {
     printf("%s <insert|delete|search> <word>\n", progname);
     printf("   to insert, delete, or find the passed English word in the dictionary\n");
     printf("or:\n");
     printf("%s <save|quit>\n", progname);
     printf("   to save the dictionary to file or quit the dictionary application\n");
}
