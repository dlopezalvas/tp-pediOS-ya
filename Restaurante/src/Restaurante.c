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
	    log_oficial = log_create("conexiones.log", "log_oficial", bool_log_oficial, LOG_LEVEL_DEBUG);

	    //  set configs:
	     if (argc != 2) {
	         log_debug(log_config_ini, "[CONFIG_INI]: Config path mal pasado, se usara default.config por defecto.");
	         config = config_create("default.config");
	     } else {
	         // cargar config desde path provisto mediante argv[1]
	         config = config_create(argv[1]);
	     }

//CONFIG
char* ip;
char* puerto;

ip= config_get_string_value(config, "IP_SINDICATO");
puerto=config_get_string_value(config, "PUERTO_SINDICATO");

log_info(log_config_ini ,ip);
log_info(log_config_ini ,puerto);
//FIN CONFIG


//ENVIAR MENSAJE
char* mensaje;
mensaje="shared library";
int socket;
socket=1234;
//enviar_mensaje(mensaje, socket);

log_info(log_config_ini ,"paso el enviar mje");






	return EXIT_SUCCESS;
}
