/** This program is the command line client program for dictionary management.
  * It is invoked with: 
  *     "dict <insert|delete|search> <word>"
  * where <word> is an English-language word.
  * Also supported are:
  *     "dict <save|quit>"
  * where quit causes the server to exit and save saves the dictionary to a predefined file.
  **/
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
 
using namespace std;

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27027"    // this is the dictionary server port

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
        reqlen = strlen(argv[1]) + strlen(argv[2]) + 1;
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
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    // Send the request to the server
    iResult = send( ConnectSocket, reqbuf, reqlen, 0 );
    if (iResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // shutdown the connection since no more data will be sent
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // now wait for the response
        // Receive until the peer closes the connection
    int rval = 1;
    do {

        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if ( iResult > 0 ) {
            recvbuf[iResult] = '\0'; // we know that the response will be shorter than recvbuf length
            printf(recvbuf);
            if (strcmp(recvbuf,"failed") == 0) {
                rval = -1;
                printf("Request failed\n");
            }
            else if (strcmp(recvbuf,"success") == 0) {
                rval = 0;
                printf("Request succeeded\n");
            }
        }
        else if ( iResult == 0 ) {
            printf("Connection closed\n");
            if (rval == 1)
                rval = -1;      // closed without getting an answer
        }
        else {
            printf("recv failed with error: %d\n", WSAGetLastError());
            rval = -1;
        }
    } while( iResult > 0 );

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();
    return rval;
}

void usage(char* progname) {
     printf("%s <insert|delete|search> <word>\n", progname);
     printf("   to insert, delete, or find the passed English word in the dictionary\n");
     printf("or:\n");
     printf("%s <save|quit>\n", progname);
     printf("   to save the dictionary to file or quit the dictionary application\n");
}