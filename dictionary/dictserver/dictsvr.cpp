/** This server program is used to manage a dictionary of English words
 * Words may be added, removed, and searched for.
 * The dictionary is saved to a flat file upon normal exit and
 * restored on restart.
 * The dictionary is managed through receiving requests on a TCP/IP socket.
 * The requests are then put in a queue to be concurrently handled by a
 * thread pool.
 **/
#include <map>
#include <iostream>
#include <cstdio>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include "dictmgr.hpp"
#include "qhandler.hpp"

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 27027

int main(int argc, char* argv[]) {
	DictMgr *mgr = new DictMgr(std::string("dictionary"));
	QHandler *handler = new QHandler(mgr);
	SOCKET listenSock;
	struct sockaddr_in serv_addr;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	std::cout << "Dictionary initialized... establishing communications" << std::endl;

	// we're initialized, so now just set up a channel and wait for requests
	listenSock = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSock < 0) {
		perror("Error opening socket");
		return listenSock;
	}

	bzero((void*)&serv_addr, sizeof(serv_addr));

	/* setup the host_addr structure for use in bind call */
	// server byte order
	serv_addr.sin_family = AF_INET;

	// automatically be filled with current host's IP address
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	// convert short integer value for port must be converted into network byte order
	serv_addr.sin_port = htons(DEFAULT_PORT);

	if (bind(listenSock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		perror("ERROR on binding");
		return -1;
	}

	listen(listenSock, SOMAXCONN);

	while (!handler->isFinished()) {

		// Accept a client socket
		SOCKET clientSock = accept(listenSock, NULL, NULL);
		if (clientSock < 0)
		{
			perror("accept failed");
			continue;
		}
		std::string req;
		while (true) {
			int n = recv(clientSock, recvbuf, recvbuflen - 1, 0);
			std::ostringstream oss;
			if (n > 0) {
				bool incomplete = false;
				if (recvbuf[n-1] != '\0') {
					recvbuf[n] = '\0';
					incomplete = true;
				}
				req += std::string(recvbuf);
				if (!incomplete)
					break;
				continue;

			} else {
				printf("incomplete message received from client");
				close(clientSock);
				clientSock = -1;
				break;
			}
		}
		if (clientSock != -1) {  // we got a full message
			// A good message should be a string with an action and optionally a word
			// Note that clientSock will be closed by handler.
			std::istringstream reqstr(req);
			std::string action;
			std::string word;
			getline(reqstr, action, ' ');
			getline(reqstr, word, ' ');
			QHandler::ACTION eact;
			if (action == "insert") {
				eact = QHandler::ACTION::INSERT;
			} else if (action == "delete") {
				eact = QHandler::ACTION::REMOVE;
			} else if (action == "search") {
				eact = QHandler::ACTION::SEARCH;
			} else if (action == "save") {
				eact = QHandler::ACTION::SAVE;
			} else if (action == "quit") {
				eact = QHandler::ACTION::QUIT;
			} else {
				eact = QHandler::ACTION::INVALID;
			}
			handler->queueReq(eact, clientSock, word);
		}
		continue;
	}
	return 0;
}

