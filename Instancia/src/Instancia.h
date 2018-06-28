#ifndef INSTANCIA_H_
#define INSTANCIA_H_

#include "configInstancia.h"
#include "../../libs/socketClient.c"
#include "../../libs/protocolo.c"


int SOCKET_COORDINADOR;
int CANTIDAD_ENTRADAS;
int TAMANIO_ENTRADAS;


typedef struct DatoEntrada{
	int numeroEntrada;
	char* clave[40];
	int tamanio;
	int* posicionEnMemoria;
} DatoEntrada;

#endif
