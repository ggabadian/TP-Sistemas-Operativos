//se usan dos sockets,uno para quedarse escuchando y otro para contestar
#include "Coordinador.h"

/*enum keys {
	IP_PROPIA, PUERTO_PROPIO, algoritmo, CANTIDAD_ENTRADAS, TAMANIO_ENTRADAS, RETARDO
};
char* keysConfigCoordinador[] = { "IP_PROPIA", "PUERTO_PROPIO", "algoritmo", "CANTIDAD_ENTRADAS", "TAMANIO_ENTRADAS", "RETARDO",
		NULL };
char* datosConfigCoordinador[6];
char* nameArchivoConfig = "configCoordinador.txt";
*/
int main() {
	const int PUERTOPROPIO = 5001;
	puts("Soy el Coordinador");
	//lee el archivo de configuracion
	//leerArchivoConfig(nameArchivoConfig, keysConfigCoordinador, datosConfigCoordinador);

	//inicializa como server
	int listener = iniciarServidor(PUERTOPROPIO);

	while (1) {
		//creo el segundo socket para recibir mensajes
		struct sockaddr_storage cliente;
		unsigned int addres_size = sizeof(cliente);
		printf("esperando al cliente\n");
		int connect = accept(listener, (struct sockaddr*) &cliente,
				&addres_size); //syscall bloqueante
		if (connect == -1) {
			printf("no se puede conectar socket secundario\n");
		}
		printf("Atendiendo al cliente\n");

		char* buffer = malloc(40); //buffer donde voy a almacenar lo que reciba
		int bytesRecibidos = recv(connect,(void*) buffer, 10, 0); //MSG_WAITALL
		if (bytesRecibidos <= 0) { //devuelve negativo si hay un error del otro lado de la conexion
			perror("el cliente se desconecto");
			return 1;
		}
		printf("me llegaron %d bytes con %s\n", bytesRecibidos, buffer);
		free(buffer);

		close(connect);
	}
	return 0;
}
