#include "Instancia.h"
#include <string.h>

bool conectarCoordinador(){
	//conecta mediante un socket la instancia con el coordinador
	puts("Conectando al coordinador");
/*
	SOCKET_COORDINADOR = connectSocket(IP_COORDINADOR, PUERTO_COORDINADOR);
	send(SOCKET_COORDINADOR, INSTANCIA, 4, 0); // Le avisa que es una INSTANCIA
*/
	return true;
}

bool enviarACoordinador(char* msg){
	int nAEnviar= strlen(msg);
	int nEnviados = send(SOCKET_COORDINADOR, msg, nAEnviar, 0);

	if(nAEnviar == nEnviados){
		return true;
	}
	return false;
}

bool recibirDeCoordinador(char* accion, char* dato){
	//debe recibir accion y dato por referencia
	// se encarga de cargar estos mediante la lectura del socket
	char* encabezado;
	int largo;

	//lectura del header
	int nRecibidos = recv(SOCKET_COORDINADOR, encabezado, 10, MSG_WAITALL);
	if (nRecibidos == 10){
		accion = string_substring(encabezado, 0, 4);      //4 primero bytes para identificar la accion
		largo = (int) string_substring(encabezado, 4, 6); //6 siguientes bytes para el tamaño del dato a leer

		//lectura del dato
		nRecibidos = recv(SOCKET_COORDINADOR, dato, largo, MSG_WAITALL);
		if(nRecibidos == largo){
			return true;
		}else{
			return false;
		}
	}else{
		return false;
	}
}

bool recibirDeCoordinadorCargaInicial(int *cantidadEntradas,int *tamanioEntradas){
	//aca llamaria a recibir coordinador posta que es el que se comunica
	char* accion = "4000";
	char* dato = "CIRC|/home/utnso/instancia1|instancia1|10|10|40";
	//dato: algoritmo|punto de montaje|id instancia| intervalo de dump| cantidad de entradas|tamaño de entrada
	char** datos;
	if(accion == "4000"){
		datos = string_split(dato, "|");
		ALGORITMO         = datos[0];
		PUNTO_MONTAJE     = datos[1];
		NOMBRE            = datos[2];
		INTERVALO_DUMP    = atoi (datos[3]);
		*cantidadEntradas = atoi (datos[4]);
		*tamanioEntradas  = atoi (datos[5]);
	}
	printf("-Algoritmo: %s "             , ALGORITMO     );
	printf("-Punto de montaje: %s \n"    , PUNTO_MONTAJE );
	printf("-Nombre: %s \n"              , NOMBRE        );
	printf("-Intervalo de dump: %d \n"   , INTERVALO_DUMP);
	printf("-Cantidad de entradas: %d \n", *cantidadEntradas);
	printf("-Tamaño de entradas: %d \n"  , *tamanioEntradas);
	return true;
}

bool cargaInicial(){
	//obitiene todos los parametros necesarios para su configuracion,
	//por parte del coordinador
	int cantidadEntradas;
	int tamanioEntradas;
	puts("Realizando carga inicial. Se cargan todas las configuraciones y tablas");

	recibirDeCoordinadorCargaInicial(&cantidadEntradas, &tamanioEntradas);

	printf("-Cantidad de entradas: %d \n", cantidadEntradas);
	printf("-Tamaño de entradas: %d \n"  , tamanioEntradas);

	t_list *listaDeEntradas = list_create();



	/*char* memoria = (char *)calloc(cantidadEntradas, tamanioEntradas);

	if (*memoria == NULL){
		printf("No se puedo asignar");
	}else{
		char* unaPalabra = string_new();
		char* otraPalabra = string_new();
		string_append(&unaPalabra, "Primer elemento");
		string_append(&otraPalabra, "Segunto elemento");
		strcpy(*memoria,*unaPalabra);
		strcpy(*memoria+1,*otraPalabra);
	}*/
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

}

void compactar(){
	//realiza la compactacion de los datos en la memoria

}

void dumping(){
	//baja a disco todos los datos que fueron seteados en la memoria hasta el momento

}

void finCompactacion(){
	//cambia el estado de Compactando a Disponible

}

void finDumping(){
	//cambia el estado de Dumping a Disponible

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
			puts("No conectada, se presenta al coordinador");
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
