#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <unistd.h>

/*
int leerArchivoConfig(char *nameArchivoConfig, char **keysConfig, char **datosConfig) {
	int i;
	char *pathArchivoConfig= string_new();
	string_append_with_format(&pathArchivoConfig, "%s", nameArchivoConfig);
	t_config *archivoConfig = config_create(pathArchivoConfig);
	free(pathArchivoConfig);
	if (!archivoConfig) {
		printf("Error: No se encuentra el archivo\nEjecución abortada\n");
		free(archivoConfig);
		return EXIT_FAILURE;
	}
	i=0;
	while(keysConfig[i]){
		datosConfig[i] = config_get_string_value(archivoConfig, keysConfig[i]);
		i++;
	}
	free(archivoConfig);
	return EXIT_SUCCESS;
}
*/
int openSocket() {
	int s = socket(PF_INET, SOCK_STREAM, 0);
	if (s == -1) {
		puts("error al abrir socket");
	}
	return s;
}
void bindToPort(int socket, int port) {
	struct sockaddr_in nombre;
	nombre.sin_family = PF_INET;
	nombre.sin_port = (in_port_t) htons(port);
	nombre.sin_addr.s_addr = htonl(INADDR_ANY);

	int reus = 1;
	if (setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (char*) &reus, sizeof(int))
			== -1) {
		perror("no es posible reusar el socket");
	}
	int c = bind(socket, (struct sockaddr*) &nombre, sizeof(nombre));
	if (c == -1) {
		perror("no se puede enlazar al puerto: direccion ya esta en uso\n");
	}
}
int iniciarServidor(const int PUERTO){
	int listener = openSocket();

	bindToPort(listener, PUERTO);

	if (listen(listener, 10) == -1) { //10 es el numero máximo de clientes que puede escuchar este socket
		printf("no es posible escuchar en ese puerto\n");
		return 1;
	}
	printf("enlazado al puerto\n");
	return listener;
}
