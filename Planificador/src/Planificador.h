
#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <commons/log.h>

#include "../../libs/socketServer.c"

void consola();

char* PUERTO = "5000";
#define BACKLOG 5				//(Pendiente) Carga desde config
#define PACKAGESIZE 1024


#endif
