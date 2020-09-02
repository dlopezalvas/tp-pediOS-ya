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
	configurar_ip_puerto(string_to_upper(argv[1])); //argv[1] = proceso al que se conecta

	pthread_t pruebaConexion;
	pthread_create(&pruebaConexion, NULL, (void*)iniciarConexion, NULL);
	pthread_join(pruebaConexion, NULL);


	iniciar_consola();



	return EXIT_SUCCESS;
}
