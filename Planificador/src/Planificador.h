
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

void consola();

#define PUERTO "5000"
#define BACKLOG 5
#define PACKAGESIZE 1024


#endif
