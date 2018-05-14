#include "configCoordinador.h"

int newSocket() {

	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(NULL, PUERTO, &hints, &serverInfo);

	int newSocket;
	newSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol);

	bind(newSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo);

	return newSocket;
}

void cargarConfig() {
	t_config *configCoord = config_create(PATH_COORDINADOR_CONFIG);

	IP = strdup(config_get_string_value(configCoord, "IP"));
	PUERTO = strdup(config_get_string_value(configCoord, "PUERTO"));
	BACKLOG = config_get_int_value(configCoord,"BACKLOG");
	PACKAGESIZE = config_get_int_value(configCoord, "PACKAGESIZE");

	config_destroy(configCoord);
}
