#ifndef COORDINADOR_H_
#define COORDINADOR_H_

#include "configCoordinador.h"
#include "../../libs/socketServer.h"
#include "../../libs/protocolo.h"
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <commons/collections/list.h>
#include <commons/log.h>

void crearThread(e_context, int);
void* threadPlanificador(void*);
void* threadESI(void*);
void* threadInstancia(void*);
void recibirMensaje(int);
void sendInitInstancia(int);
void registrarInstancia(int);


typedef struct {
	int socket;
	int entradasLibres;
	//lista de claves
} __attribute__((packed)) t_instancia;


#endif
