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
#include <commons/collections/dictionary.h>
#include <commons/log.h>
#include <math.h>

#define ERROR 1

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Las instancias que se conectan se guardan en esta lista
t_list *instanciasRegistradas;

int socketPlanificador = 0;
int socketConsola = 0;

// Esto registra el indice de la ultima instancia elegida
int indexEquitativeLoad = 0;

t_log* logCoordinador;
t_log* logDeOperaciones;

int instanciasCompactando = 0;

typedef struct {
	char* nombre;
	int socket;
	int entradasLibres;
} t_instancia;

t_dictionary *clavesRegistradas; // CLAVE::INSTANCIA

void crearThread(e_context, int);
void* threadPlanificador(void*);
void* threadConsola(void*);
void* threadESI(void*);
void* threadInstancia(void*);
void recibirMensaje(int);
void sendInitInstancia(int);
void registrarInstancia(int, char*);
t_instancia* instanciaRegistrada(char*);
bool distribuirSet(t_set);
t_instancia* equitativeLoad();
t_instancia* leastSpaceUsed();
t_instancia* keyExplicit(char*);
void enviarSet(t_instancia*, t_set);
bool distribuirStore(t_store);
void enviarStore(t_instancia*, t_head, t_store);
void sendBlockedESI(int);
void sendOkESI(int);
void sendAbortESI(int);
bool desconectado (int);
void enviarOrdenCompactar();
t_list *instanciasActivas();
t_instancia* instanciaConClave(char*);
bool claveRegistrada(char*, t_instancia*);
t_instancia* instanciaConSocket(int);

#endif
