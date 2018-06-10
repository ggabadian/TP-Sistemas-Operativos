
#ifndef LIBS_PROTOCOLO_H_
#define LIBS_PROTOCOLO_H_

#include <sys/socket.h>

char* identificar(int);
int recibirHead(int);
void enviarHead(int, int);

#endif /* LIBS_PROTOCOLO_H_ */
