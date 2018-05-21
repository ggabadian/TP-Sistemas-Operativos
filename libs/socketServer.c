// Funciones de sockets para el rol de SERVIDOR

#include "socketServer.h"

int listenSocket(char *puerto) {

	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(NULL, puerto, &hints, &serverInfo);

	int newSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

	bind(newSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);

	freeaddrinfo(serverInfo);

	return newSocket;
}

int acceptSocket(int listenerSocket) {
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	return accept(listenerSocket, (struct sockaddr *) &addr, &addrlen);
}
