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

void ejecucion_guardar_pedido();
void ejecucion_guardar_plato();
void ejecucion_finalizar_pedido();
void ejecucion_confirmar_pedido();
void ejecucion_plato_listo();
void ejecucion_obtener_pedido();

#endif /* UTILS_COMANDA_H_ */
