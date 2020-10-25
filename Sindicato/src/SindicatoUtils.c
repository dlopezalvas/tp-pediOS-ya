#include "SindicatoUtils.h"

/* ********************************** PUBLIC  FUNCTIONS ********************************** */

t_log* sindicato_utils_iniciar_debug_logger(t_config* config){
	char* nombre_archivo = config_get_string_value(config,"DEBUG_LOG_FILE");
	char* nombre_aplicacion = config_get_string_value(config,"DEBUG_NOMBRE_APLICACION");
	int log_consola = config_get_int_value(config,"DEBUG_LOG_CONSOLA");

	t_log* logger = log_create(nombre_archivo,nombre_aplicacion,log_consola,LOG_LEVEL_INFO);
	return logger;
}

void sindicato_utils_create_folder(char* path, bool logsFolder){
	struct stat st = {0};

	/* Validate if the folder exists to create the folder */
	if(stat(path, &st) == -1){
		if(mkdir(path,0777) == 0){
			if(!logsFolder)
				log_info(sindicatoDebugLog, "[FILESYSTEM] Carpeta creada: %s",path);
		}
	}else{
		if(!logsFolder)
			log_info(sindicatoDebugLog, "[FILESYSTEM] Carpeta existente: %s", path);
	}
}

char* sindicato_utils_build_path(char* path, char* toAppend){
	char* pathBuilded = string_duplicate(path);
	string_append(&pathBuilded, toAppend);
	return pathBuilded;
}
