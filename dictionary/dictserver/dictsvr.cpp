/** This server program is used to manage a dictionary of English words
  * Words may be added, removed, and searched for.
  * The dictionary is saved to a flat file upon normal exit and
  * restored on restart.
  * The dictionary is managed through receiving requests on a TCP/IP socket.
  * The requests are then put in a queue to be concurrently handled by a 
  * thread pool.
  **/
  #include <map>
  #include <cstdio>
  #include <Winsock2.h>
  #include <ws2tcpip.h>
  #include <sstream>
  #include "dictmgr.hpp"
  #include "qhandler.hpp"

  #define DEFAULT_BUFLEN 512
  #define DEFAULT_PORT "27027"

  int main() {
      DictMgr *mgr = new DictMgr(std::string("dictionary"));
      QHandler *handler = new QHandler(mgr);
      SOCKET listenSock;

      // we're initialized, so now just set up a channel and wait for requests
      struct addrinfo *result = NULL;
      struct addrinfo hints;

      int iSendResult;
      char recvbuf[DEFAULT_BUFLEN];
      int recvbuflen = DEFAULT_BUFLEN;
      WSADATA wsaData;
      int iResult;


      // Initialize Winsock
      iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
      if (iResult != 0)
      {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
      }

      ZeroMemory(&hints, sizeof(hints));
      hints.ai_family = AF_INET;
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_protocol = IPPROTO_TCP;
      hints.ai_flags = AI_PASSIVE;

      // Resolve the server address and port
      iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
      if (iResult != 0)
      {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
      }

      // Create a SOCKET for connecting to server
      listenSock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

          // Setup the TCP listening socket
    iResult = bind( listenSock, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(listenSock);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    while (!handler->isFinished()) {
      iResult = listen(listenSock, SOMAXCONN);
      if (iResult == SOCKET_ERROR)
      {
        printf("listen failed with error: %d\n", WSAGetLastError());
        continue;
      }

      // Accept a client socket
      SOCKET clientSock = accept(listenSock, NULL, NULL);
      if (clientSock == INVALID_SOCKET)
      {
        printf("accept failed with error: %d\n", WSAGetLastError());
        continue;
      }
      std::string req;
      while (true) {
        iResult = recv(clientSock, recvbuf, recvbuflen - 1, 0);
        if (iResult > 0) {
          if (recvbuf[iResult-1] != '\0') {
            recvbuf[iResult] = '\0';
            req += std::string(recvbuf);
            continue;
          }
          break;
        } else {
          printf("incomplete message received from client");
          closesocket(clientSock);
          clientSock = INVALID_SOCKET;
          break;
        }
    }
    if (clientSock != INVALID_SOCKET) {  // we got a full message
      // A good message should be a string with an action and optionally a word
      // Note that clientSock will be closed by handler.
      std::stringstream reqstr(req);
      std::string action;
      std::string word;
      getline(reqstr, action, ' ');
      getline(reqstr, word, ' ');
      QHandler::ACTION eact;
      if (action == "insert") {
        eact = QHandler::ACTION::INSERT;
        break;
      } else if (action == "delete") {
        eact = QHandler::ACTION::REMOVE;
        break;
      } else if (action == "search") {
        eact = QHandler::ACTION::SEARCH;
        break;
      } else if (action == "save") {
        eact = QHandler::ACTION::SAVE;
        break;
      } else if (action == "quit") {
        eact = QHandler::ACTION::QUIT;
        break;
      } else {
        eact = QHandler::ACTION::INVALID;
        break;
      }
      handler->queueReq(eact, clientSock, word);
    }
    continue;
  }
