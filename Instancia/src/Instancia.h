
#ifndef INSTANCIA_H_
#define INSTANCIA_H_

#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <commons/string.h>
#include <commons/log.h>
#include "../../libs/socketClient.c"
#include "../../libs/protocolo.c"


#define IP_COORDINADOR "127.0.0.1"
#define PUERTO_COORDINADOR "8000"

int SOCKET_COORDINADOR;
int CANTIDAD_ENTRADAS;
int TAMANIO_ENTRADAS;
char* NOMBRE;
char* PUNTO_MONTAJE;
char* ALGORITMO;
int INTERVALO_DUMP;


typedef struct DatoEntrada{
	int numeroEntrada;
	int tamanio;
} DatoEntrada;

#endif
