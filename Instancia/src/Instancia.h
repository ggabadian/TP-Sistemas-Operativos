#ifndef INSTANCIA_H_
#define INSTANCIA_H_

#include "configInstancia.h"
#include "../../libs/socketClient.h"
#include "../../libs/protocolo.h"


int SOCKET_COORDINADOR;
int CANTIDAD_ENTRADAS;
int TAMANIO_ENTRADAS;
t_list* TABLA_ENTRADAS;

t_log* LOG_INSTANCIA;

t_set paqueteSet;
t_store paqueteStore;
t_initInstancia paqueteInit;
int connected;


void enviarACoordinador(char* msg);
void conectarCoordinador();
void recibirOperacion();
int main(int argc, char* argv[]);
void realizarSet();

typedef struct {
	char clave[40];
	int posicion;
	int tamanio;
	int cantidad;
}datoEntrada;

#endif
