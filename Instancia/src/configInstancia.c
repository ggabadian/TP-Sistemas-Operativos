#include "configInstancia.h"

void cargarConfig(char* path) {
	t_config *configInstancia = config_create(path);

	IP_COORDINADOR = strdup(config_get_string_value(configInstancia, "IP_COORDINADOR"));
	PUERTO_COORDINADOR = strdup(config_get_string_value(configInstancia, "PUERTO_COORDINADOR"));
	ALGORITMO = strdup(config_get_string_value(configInstancia, "ALGORITMO"));
	PUNTO_MONTAJE = strdup(config_get_string_value(configInstancia, "PUNTO_MONTAJE"));
	NOMBRE = strdup(config_get_string_value(configInstancia, "NOMBRE"));
	INTERVALO_DUMP = config_get_int_value(configInstancia,"INTERVALO_DUMP");

	config_destroy(configInstancia);
}

