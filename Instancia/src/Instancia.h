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

void set(t_set *paqueteSet);
void store(t_store *paqueteStore);
bool enviarACoordinador(char* msg);
bool conectarCoordinador();
bool recibirDeCoordinador();
void respuesta();
void compactar();
void dumping();
void finCompactacion();
void finDumping();
int main(int argc, char* argv[]);

typedef struct {
	char clave[40];
	char* valor;
	int tamanio;
	int cantidadOperaciones;
}datoEntrada;

#endif
