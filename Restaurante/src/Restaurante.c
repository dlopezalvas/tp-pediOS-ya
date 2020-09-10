/*
 ============================================================================
 Name        : Restaurante.c
 Author      : 
 Version     :
 Copyright   : 
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "Restaurante.h"


int main(int argc, char* argv[]) {

// ACTIVACION DE CONSOLA POR TIPO DE LOG:
	  	bool_log_oficial    = false;
	    bool_log_restaurante = true;


	    //  set logs no oficiales
	    log_config_ini = log_create("log_restaurante.log", "restaurante", bool_log_restaurante, LOG_LEVEL_DEBUG);
	    log_oficial = log_create("log_oficial.log", "log_oficial", bool_log_oficial, LOG_LEVEL_DEBUG);

	    //  set configs:
	     if (argc != 2) {
	         log_debug(log_config_ini, "[CONFIG_INI]: Config path mal pasado, se usara default.config por defecto.");
	         config = leer_config("/home/utnso/workspace/tp-2020-2c-CoronaLinux/Restaurante/src/default.config");
	     } else {
	         // cargar config desde path provisto mediante argv[1]
	         config = config_create(argv[1]);
	     }
// FIN ACTIVACION DE CONSOLA POR TIPO DE LOG:

puerto=config_get_int_value(config,"PUERTO_SINDICATO");
//puerto =5002;
ip=config_get_string_value(config,"IP_SINDICATO");

log_info(log_config_ini ,ip);
//log_info(log_config_ini ,puerto);


iniciar_restaurante(ip,puerto);





	return EXIT_SUCCESS;
}
