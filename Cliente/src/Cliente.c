/*
 ============================================================================
 Name        : Cliente.c
 Author      : 
 Version     :
 Copyright   : 
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "Cliente.h"

int main(int argc, char* argv[]) {
	config_cliente = leer_config(PATH);
	log_cliente = iniciar_logger(config_cliente);
	if(argc !=2 || !validar_proceso(argc, argv)){
		puts("Por favor ingrese alguno de los siguiente parametros al ejecutar: \nComanda\nApp\nRestaurante\nSindicato");
		return EXIT_SUCCESS;
	}

	proceso = string_duplicate(argv[1]);
	string_to_upper(proceso);
	configurar_ip_puerto();




	pthread_t conexion1;
	pthread_create(&conexion1, NULL, (void*)conexionEnvio, NULL);
	pthread_join(conexion1, NULL);

	pthread_t conexion2;
	pthread_create(&conexion2, NULL, (void*)conexionEnvio, NULL);
	pthread_join(conexion2, NULL);


	iniciar_consola();



	return EXIT_SUCCESS;
}
