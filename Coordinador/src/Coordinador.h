#ifndef COORDINADOR_H_
#define COORDINADOR_H_

#include "configCoordinador.h"
#include "../../libs/socketServer.h"
#include "../../libs/protocolo.h"
#include <pthread.h>
#include <stdint.h>

int socketPlanificador;
int socketESI;
int socketInstancia;

typedef struct {
	uint32_t cantidadEntradas;
	uint32_t sizeofEntrada;
} t_InitInstancia; // __attribute__((packed));

void crearThread(int, int);
void* threadPlanificador(void*);
void* threadESI(void*);
void* threadInstancia(void*);
void recibirMensaje(int);
void sendInitInstancia(int, int, int);
void freePackage(char **);

#endif
