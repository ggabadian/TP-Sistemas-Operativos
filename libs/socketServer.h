#ifndef LIBS_SOCKETSERVER_H_
#define LIBS_SOCKETSERVER_H_

#include <netdb.h>
#include <string.h>
#include <sys/socket.h>

int listenSocket(char*);
int acceptSocket(int);

#endif /* LIBS_SOCKETSERVER_H_ */
