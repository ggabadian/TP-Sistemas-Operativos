
#include "configPlanificador.h"

void cargarConfigPlanificador() {
	t_config *configPlanificador = config_create(PATH_PLANIFICADOR_CONFIG);

	PUERTO = strdup(config_get_string_value(configPlanificador, "PUERTO"));
	ALGORITMO = strdup(config_get_string_value(configPlanificador, "ALGORITMO"));
	ALFA = config_get_int_value(configPlanificador,"ALFA");
	ESTIMACION_I = config_get_int_value(configPlanificador, "ESTIMACION_I");
	IP_COORDINADOR = strdup(config_get_string_value(configPlanificador, "IP_COORDINADOR"));
	PUERTO_COORDINADOR = strdup(config_get_string_value(configPlanificador,"PUERTO_COORDINADOR"));
	CL_BLOQUEADAS = config_get_array_value(configPlanificador, "CL_BLOQUEADAS");

	config_destroy(configPlanificador);
}


