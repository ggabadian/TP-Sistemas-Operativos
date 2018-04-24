#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main(void) {
	puts("hola soy el cliente"); /* prints !!!Hello World!!! */

	//creo la estructura con la direccion del servidor
	struct sockaddr_in direccionDelServidor;
	direccionDelServidor.sin_family = AF_INET;
	direccionDelServidor.sin_addr.s_addr = inet_addr("127.0.0.1");
	direccionDelServidor.sin_port = htons(5000);

	int cliente = socket(AF_INET, SOCK_STREAM, 0);//guardo el id del socket del cliente
	//me intento conectar, si falla la conexion(<0) manejo el error
	if (connect(cliente, (void*) &direccionDelServidor, sizeof(direccionDelServidor)) != 0){
		perror("No se pudo conectar");
	}
	while(1){
		char mensaje[1000];
		scanf("%s",mensaje);
		send(cliente,mensaje,strlen(mensaje),0);
	}



	return 0;
}
