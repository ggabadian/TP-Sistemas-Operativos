#ifndef LIBS_SOCKETCLIENT_H_
#define LIBS_SOCKETCLIENT_H_

#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int connectToServer(char*, char*);
int connectSocket(char*, char*);

#endif /* LIBS_SOCKETCLIENT_H_ */
