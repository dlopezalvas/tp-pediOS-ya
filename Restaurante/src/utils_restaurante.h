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
#include <semaphore.h>
#include <commons/string.h>
#include<commons/log.h>
#include<commons/config.h>
#include <../commonsCoronaLinux/utils.h>
#include <commons/collections/queue.h>
#include <../commonsCoronaLinux/socket.h>
#include <../commonsCoronaLinux/logs.h>
#include <inttypes.h>
#include <errno.h>

#include <pthread.h>
#include <sys/socket.h>

#define REPOSAR "REPOSAR"
#define HORNEAR "HORNEAR"

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
int cfg_id;

void conectarme_con_app();

int socket_app;


//LOG
t_log* log_oficial;
t_log* log_config_ini;


//INICIAR RESTAURANTE
void iniciar_restaurante();
void* fhilo_servidor_clientes(void* v);
void esperar_cliente(int servidor);
void serve_client(int socket);
void process_request(int cod_op, int cliente_fd);
void* recibir_respuesta(int socket);
int conectar_con_sindicato();
void inicio_de_listas_globales();
bool mismo_nombre(t_nombre* afinidad1, t_nombre* afinidad2);
void enviar_confirmacion(uint32_t _confirmacion, int cliente, op_code cod_op);


//HILOS - SEMAFOROS
pthread_t hilo_servidor_clientes;
pthread_t hilo_planificador;
t_list *  hilos;
t_list * hilos_reposo;
pthread_mutex_t cola_afinidades_mtx;
t_list* colas_afinidades;
sem_t hornos_disp;
sem_t platos_a_hornear_sem;
pthread_mutex_t platos_block_mtx;
t_list* platos_block;
t_list* platos_exec;
pthread_mutex_t platos_exec_mtx;

pthread_mutex_t mutex_hilos;
pthread_mutex_t mutex_id_pedidos;

pthread_mutex_t mutex_pcb;
t_list *  pcb_platos;

pthread_t hilo_serve_app;
typedef enum{
	NEW = 0,
	READY = 1,
	BLOCK = 2,
	EXEC = 3,
	EXIT = 4,
}est_planif;

//PCB - PLATOS
typedef struct{
			uint32_t id_pedido;
			t_nombre comida;
			uint32_t cantPasos;
			t_list* pasos;//t_paso
			uint32_t cantTotal;
			uint32_t cantHecha;
			est_planif estado;
}t_plato_pcb;


typedef struct{
			t_nombre afinidad;
			t_queue* ready; //cola de platos
			pthread_mutex_t cola_ready_mtx;
			sem_t sem_exec; //para saber si hay platos en cola_cocineros_exec
			sem_t cocineros_disp_sem; //para saber si hay cocineros disp
			sem_t platos_disp; //para saber si hay platos en ready
			uint32_t cant_cocineros_disp;
			t_queue* cola_cocineros_disp; //cola de t_cocinero
			pthread_mutex_t cola_cocineros_disp_mtx;
			t_queue* cola_cocineros_exec;
			pthread_mutex_t cola_cocineros_exec_mtx;
}t_cola_afinidad;

typedef struct{
		t_plato_pcb* plato_a_cocinar;
		pthread_mutex_t mtx_exec; //para que ejecute el cocinero
		pthread_t hilo;
}t_cocinero;



typedef struct{
	t_nombre cocinero;
	t_nombre afinidad;
}t_cocinero_afinidad;


void* fhilo_plato (t_plato_pcb* v);

//FHILOS
void* fhilo_planificador (void* v);
void* fhilo_serve_app (int socket);


//METADATA
rta_obtenerRestaurante* metadata_rest;
void iniciar_colas_ready_es();
void delay (int number_of_seconds);
int id_pedidos;



//LISTAS

t_list *  list_pedidos;


//MENSAJES
uint32_t recibir_RTA_GUARDAR_PEDIDO(int socket);
uint32_t recibir_RTA_GUARDAR_PLATO(int socket);
rta_obtenerPedido* recibir_RTA_OBTENER_PEDIDO (int socket);
rta_obtenerReceta* recibir_RTA_OBTENER_RECETA(int socket);

//FUNCIONES
t_restaurante_y_plato* recibir_RTA_CONSULTAR_PLATOS(int socket);
void agregar_cola_ready(t_plato_pcb* plato);
char* stringEstado(est_planif estado);
bool cambioEstadoValido(est_planif estadoViejo,est_planif nuevoEstado);
void cambiarEstado (t_plato_pcb* plato, est_planif nuevoEstado);


#endif /* RESTAURANTE_SRC_UTILS_RESTAURANTE_H_ */
