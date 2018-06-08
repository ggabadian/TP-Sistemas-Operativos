// Funciones de sockets para el rol de CLIENTE

#include "socketClient.h"

int connectToServer(char *ip, char *puerto){

	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(ip, puerto, &hints, &serverInfo);

	int newSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype,serverInfo->ai_protocol);

	if ((connect(newSocket, serverInfo->ai_addr, serverInfo->ai_addrlen)) < 0){
		freeaddrinfo(serverInfo);
		return -1; // No se pudo conectar
	}

	freeaddrinfo(serverInfo);

	return newSocket;
}

int connectSocket(char *ip, char *puerto){
	int serverSocket = connectToServer(ip, puerto);

	while (serverSocket < 0){ // Si no se pudo conectar, vuelve a intentarlo
		sleep(1);
		serverSocket = connectToServer(ip, puerto);
	}

	return serverSocket;
}
