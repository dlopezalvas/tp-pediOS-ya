/*
 * utils_restaurante.h
 *
 *  Created on: 1 sep. 2020
 *      Author: utnso
 */

#ifndef RESTAURANTE_SRC_UTILS_RESTAURANTE_H_
#define RESTAURANTE_SRC_UTILS_RESTAURANTE_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include<commons/log.h>
#include<commons/config.h>
#include <../commonsCoronaLinux/utils.h>
#include <../commonsCoronaLinux/socket.h>

t_config* config;
t_config* config_sindicato;

char* ip;
int puerto;

void iniciar_restaurante(char*,int);

t_log* log_oficial;
t_log* log_config_ini;
void delay (int number_of_seconds);
rta_obtenerRestaurante* metadata_restaurante(int socket);

#endif /* RESTAURANTE_SRC_UTILS_RESTAURANTE_H_ */
