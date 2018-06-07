#include "configESI.h"

void cargarConfig() {
	t_config *configESI = config_create(PATH_ESI_CONFIG);

	IP_COORDINADOR = strdup(config_get_string_value(configESI, "IP_COORDINADOR"));
	PUERTO_COORDINADOR = strdup(config_get_string_value(configESI, "PUERTO_COORDINADOR"));
	IP_PLANIFICADOR = strdup(config_get_string_value(configESI, "IP_PLANIFICADOR"));
	PUERTO_PLANIFICADOR = strdup(config_get_string_value(configESI, "PUERTO_PLANIFICADOR"));

	config_destroy(configESI);
}
