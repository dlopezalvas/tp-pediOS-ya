/*
 ============================================================================
 Name        : Comanda.c
 Author      : 
 Version     :
 Copyright   : 
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <../commonsCoronaLinux/utils.h>
#include <../commonsCoronaLinux/socket.h>
#include "Comanda.h"


int main(void) {

	iniciar_comanda();
	int servidor = iniciar_servidor(config_get_int_value(config_comanda, PUERTO_ESCUCHA));

//	while(1){
//		esperar_cliente(servidor);
//	}

	return EXIT_SUCCESS;
}
