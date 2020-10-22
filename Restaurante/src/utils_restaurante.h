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
#include <inttypes.h>

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
void inicio_de_listas_globales();

//HILOS - SEMAFOROS
pthread_t hilo_servidor_clientes;
pthread_t hilo_planificador;
t_list *  hilos;
pthread_mutex_t mutex_hilos;
pthread_mutex_t mutex_id_pedidos;

pthread_mutex_t mutex_pcb;
t_list *  hilos_pcb;

//PCB - PLATOS
typedef struct{
			uint32_t id_pedido;
			t_nombre comida;
			uint32_t cantPasos;
			t_list* pasos;//t_paso
			uint32_t cantTotal;
			uint32_t cantHecha;
}t_plato_pcb;

void* fhilo_plato (t_plato_pcb* v);

//FHILOS
void* fhilo_planificador (void* v);


//METADATA
rta_obtenerRestaurante* metadata_rest;
void iniciar_colas_ready_es(rta_obtenerRestaurante* metadata);
void delay (int number_of_seconds);
int id_pedidos;


//LISTAS

t_list *  list_pedidos;
t_list *  status_platos;

//MENSAJES
uint32_t recibir_RTA_GUARDAR_PEDIDO(int socket);
uint32_t recibir_RTA_GUARDAR_PLATO(int socket);
rta_obtenerPedido* recibir_RTA_OBTENER_PEDIDO (int socket);
rta_obtenerReceta* recibir_RTA_OBTENER_RECETA(int socket);

//FUNCIONES
t_restaurante_y_plato* recibir_RTA_CONSULTAR_PLATOS(int socket);


#endif /* RESTAURANTE_SRC_UTILS_RESTAURANTE_H_ */
