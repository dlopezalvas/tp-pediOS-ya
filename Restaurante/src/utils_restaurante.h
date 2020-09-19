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

#include <pthread.h>
#include <sys/socket.h>


//CONFIGURACION
void cargar_configuracion();
t_config* config;
t_config* config_sindicato;
char* cfg_ip_sindicato;
int cfg_puerto_sindicato;
int cfg_puerto_escucha;
int cfg_puerto_app;
char* cfg_ip_app;
char* cfg_nombre_restaurante;
char* cfg_algoritmo_planificacion;
int cfg_quantum;


//LOG
t_log* log_oficial;
t_log* log_config_ini;


//INICIAR RESTAURANTE
void iniciar_restaurante();
void* fhilo_servidor_clientes(void* v);
void esperar_cliente(int servidor);
void serve_client(int socket);
void process_request(int cod_op, int cliente_fd);
rta_obtenerRestaurante* metadata_restaurante(int socket);
int conectar_con_sindicato();

//HILOS - SEMAFOROS
pthread_t hilo_servidor_clientes;
pthread_t hilo_planificador;
t_list *  hilos;
pthread_mutex_t mutex_hilos;


//FHILOS
void* fhilo_planificador (void* v);


//METADATA
rta_obtenerRestaurante* metadata_rest;
void iniciar_colas_ready_es(rta_obtenerRestaurante* metadata);
void delay (int number_of_seconds);

//LISTAS
t_list *  list_pedidos;





#endif /* RESTAURANTE_SRC_UTILS_RESTAURANTE_H_ */
