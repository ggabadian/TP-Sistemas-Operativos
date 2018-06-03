
#ifndef ESI_H_
#define ESI_H_

#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <commons/log.h>
#include "../../libs/socketClient.c"
#include "../../libs/protocolo.c"


#define IP "127.0.0.1"
#define PUERTO_COORDINADOR "5000"
#define PACKAGESIZE 1024

#endif
