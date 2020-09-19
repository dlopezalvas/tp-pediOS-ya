#ifndef UTILS_COMANDA_H_
#define UTILS_COMANDA_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include <semaphore.h>
#include <../commonsCoronaLinux/utils.h>
#include <../commonsCoronaLinux/socket.h>
#include <../commonsCoronaLinux/logs.h>

#define TAMANIO_PAGINA 32
#define TAMANIO_NOMBRE 24
#define PATH "/home/utnso/workspace/tp-2020-2c-CoronaLinux/Comanda/Comanda.config"
#define PUERTO_ESCUCHA "PUERTO_ESCUCHA"
#define TAMANIO_MEMORIA "TAMANIO_MEMORIA"
#define TAMANIO_SWAP "TAMANIO_SWAP"
#define ALGORITMO_REEMPLAZO "ALGORITMO_REEMPLAZO"

t_config* config_comanda;
t_log* log_comanda;

t_list* hilos_clientes;
pthread_mutex_t hilos_clientes_mtx;

t_list* hilos_operaciones;
pthread_mutex_t hilos_operaciones_mtx;

typedef struct{
	char* nombre;
	t_list* tabla_segmentos;
	pthread_mutex_t tabla_segmentos_mtx;
} t_restaurante;

typedef struct{
	int id_pedido;
	t_list* tabla_paginas;
} t_segmento;

typedef struct{
	void* mensaje;
	int socket_cliente;
} t_mensaje_a_procesar;

typedef struct{
	uint32_t frame;
	uint32_t ultimo_acceso; // time(NULL);
	bool uso; //se inicia en 1
	bool modificado; // se inicia en 0
}t_pagina;

typedef struct{
	uint32_t cant_pedida;
	uint32_t cant_lista;
	char nombre[24];
}t_plato;


pthread_mutex_t restaurantes_mtx;
t_list* restaurantes;


//pthread_t guardar_pedido_queue_mtx;
//pthread_t guardar_plato_queue_mtx;
//pthread_t obtener_pedido_queue_mtx;
//pthread_t confimar_pedido_queue_mtx;
//pthread_t plato_listo_queue_mtx;
//pthread_t finalizar_pedido_queue_mtx;
//
//t_queue* guardar_pedido_queue;
//t_queue* guardar_plato_queue;
//t_queue* obtener_pedido_queue;
//t_queue* confimar_pedido_queue;
//t_queue* plato_listo_queue;
//t_queue* finalizar_pedido_queue;

void* memoria_principal;
void* memoria_swap;

void iniciar_comanda();
void process_request(int cod_op, int cliente_fd);
void serve_client(int socket);
void esperar_cliente(int servidor);

t_restaurante* buscarRestaurante(char* nombre);
t_segmento* buscarPedido(uint32_t id_pedido, char* nombre);
void enviar_confirmacion(uint32_t _confirmacion, int cliente, op_code cod_op);
t_pagina* buscarPlato(t_list* tabla_paginas, char* comida);
void* serializar_pagina(t_plato* plato);

void ejecucion_guardar_pedido(t_mensaje_a_procesar* mensaje_a_procesar);
void ejecucion_guardar_plato(t_mensaje_a_procesar* mensaje_a_procesar);
void ejecucion_finalizar_pedido(t_mensaje_a_procesar* mensaje_a_procesar);
void ejecucion_confirmar_pedido(t_mensaje_a_procesar* mensaje_a_procesar);
void ejecucion_plato_listo(t_mensaje_a_procesar* mensaje_a_procesar);
void ejecucion_obtener_pedido(t_mensaje_a_procesar* mensaje_a_procesar);

#endif /* UTILS_COMANDA_H_ */
