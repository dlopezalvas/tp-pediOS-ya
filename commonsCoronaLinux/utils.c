#include "utils.h"

t_log* iniciar_logger(t_config* config)
{
	char* nombre_archivo = config_get_string_value(config,"LOG_FILE");
	char* nombre_aplicacion = config_get_string_value(config,"NOMBRE_APLICACION");
	//char* path = config_get_string_value(config,"PATH");
	int log_consola = config_get_int_value(config,"LOG_CONSOLA");

	t_log* logger = log_create(nombre_archivo,nombre_aplicacion,log_consola,LOG_LEVEL_INFO);
	return logger;
}

t_config* leer_config(char* path)
{
	t_config* config = config_create(path);
	return config;
}



void liberar_vector (char** vector){

	int i = 0;
	while(vector[i]!=NULL){
		free(vector[i]);
		i++;
	}

	free(vector);
}



