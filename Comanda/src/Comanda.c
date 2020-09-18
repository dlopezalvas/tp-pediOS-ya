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
	log_comanda = log_create("/home/utnso/workspace/tp-2020-2c-CoronaLinux/Comanda/comanda.LOG","comanda",1,LOG_LEVEL_INFO);

	int servidor = iniciar_servidor(5001);

	while(1){
		esperar_cliente(servidor);
	}

	return EXIT_SUCCESS;
}
