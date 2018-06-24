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

#define ERROR 1

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Las instancias conectadas se guardan en esta lista
t_list *instanciasConectadas;

int socketPlanificador = 0;

// Esto registra el indice de la ultima instancia elegida
int indexEquitativeLoad = 0;


typedef struct {
	int socket;
	int entradasLibres;
	//lista de claves
} __attribute__((packed)) t_instancia;

void crearThread(e_context, int);
void* threadPlanificador(void*);
void* threadESI(void*);
void* threadInstancia(void*);
void recibirMensaje(int);
void sendInitInstancia(int);
void registrarInstancia(int);
void assignSet(t_set);
t_instancia* equitativeLoad();
t_instancia* leastSpaceUsed();
t_instancia* keyExplicit();
void sendSet(t_instancia*, t_set);
void assignStore(t_head, char*);
void sendStore(t_instancia*, t_head, char*);
void sendBlockedESI(int);

#endif
