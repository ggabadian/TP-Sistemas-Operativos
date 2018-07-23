#ifndef INSTANCIA_H_
#define INSTANCIA_H_

#include "configInstancia.h"
#include "../../libs/socketClient.h"
#include "../../libs/protocolo.h"


int SOCKET_COORDINADOR;
int CANTIDAD_ENTRADAS;
int TAMANIO_ENTRADAS;
char** ALMACENAMIENTO;
t_list* TABLA_ENTRADAS;
t_bitarray BIT_ARRAY;

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
bool hayEspacioContiguo(int tamanio, int* posicion);
bool hayEntradasDisponibles(int tamanio);
int entradasNecesarias(char* valor);
void realizarStore(char clave[40]);
void correrAlgoritmoDeReemplazo();
bool yaExisteClave(char clave[40]);

typedef struct {
	char clave[40];
	int posicion;
	int tamanio;
	int cantidadUtilizada;
	int cantidadReferencias;
}datoEntrada;

#endif
