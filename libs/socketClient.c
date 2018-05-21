// Funciones de sockets para el rol de CLIENTE

#include "socketClient.h"

int connectSocket(char *ip, char *puerto){
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(ip, puerto, &hints, &serverInfo);

	int serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype,serverInfo->ai_protocol);

	connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);

	freeaddrinfo(serverInfo);

	return serverSocket;
}
