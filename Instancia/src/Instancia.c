#include "Instancia.h"
#include <stdbool.h>

bool conectarCoordinador(){
	//conecta mediante un socket la instancia con el coordinador

	puts("Conectando al coordinador");
	sleep(1);
	return true;
}

bool enviarACoordinador(int socket, char* msg){
	int nAEnviar= strlen(msg);
	int nEnviados = send(socket, msg, nAEnviar, 0);

	if(nAEnviar == nEnviados){
		return true;
	}
	return false;
}

bool recibirDeCoordinador(int socket, char* accion, char* dato){
	//debe recibir accion y dato por referencia
	char* encabezado;
	int largo;

	//lectura del header
	int nRecibidos = recv(socket, encabezado, 10, 0);
	if (nRecibidos == 10){
		accion = string_substring(encabezado, 0, 4);      //4 primero bytes para identificar la accion
		largo = (int) string_substring(encabezado, 4, 6); //6 siguientes bytes para el tamaño del dato a leer

		//lectura del dato
		nRecibidos = recv(socket, dato, largo, 0);
		if(nRecibidos == largo){
			return true;
		}else{
			return false;
		}
	}else{
		return false;
	}
}

bool cargaInicial(){
	//obitiene todos los parametros necesarios para su configuracion,
	//por parte del coordinador
	puts("Realizando carga inicial. Se cargan todas las configuraciones y tablas");
	sleep(1);
	return true;
}

bool hayNuevaSentencia(){
	//recibe del coordinador una sentencia nueva
	puts("Recibiendo nueva sentencia");
	return true;
}

void procesarSentencia(){
	//interpreta el mensaje del coordinador, y realiza la accion correspondiente
	puts("Procesando sentencia");
	sleep(1);
}

void respuesta(){
	//prepara la respueta que se le va a dar al coordinador
	sleep(1000);
}

void compactar(){
	//realiza la compactacion de los datos en la memoria
	sleep(1000);
}

void dumping(){
	//baja a disco todos los datos que fueron seteados en la memoria hasta el momento
	sleep(1000);
}

void finCompactacion(){
	//cambia el estado de Compactando a Disponible
	sleep(1000);
}

void finDumping(){
	//cambia el estado de Dumping a Disponible
	sleep(1000);
}

int main(void) {
	int estado;
	/* 0: No Conectada
	 * 1: Conectada, configurandose
	 * 2: Disponible
	 * 3: Procesando Sentencia
	 * 4: Compactando
	 * 5: Dumping
	 * */
	estado = 0;

	while(1){

		switch(estado){
		case 0:
			puts("No conectado, se presenta al coordinador");
			if (conectarCoordinador()){
				estado = 1;
			}
		break;

		case 1:
			puts("Conectada, procedera a realizar la cargaInicial");
			if (cargaInicial()){
				estado = 2;
			}
		break;

		case 2:
			puts("Disponible, esperando que el coordinador envie una sentencia");
			if (hayNuevaSentencia()){
				estado = 3;
			}
		break;

		case 3:
			puts("Procesando sentencia, se interpretara la operacion a realizar y se realizara");
			procesarSentencia();
			estado=2;
		break;

		case 4:
			puts("Compactando, se compactaran los datos");
		break;

		case 5:
			puts("Dumping, dup de datos a disco");
		break;
		}
		sleep(3);
	}

	return EXIT_SUCCESS;
}
