#ifndef UTILS_COMANDA_H_
#define UTILS_COMANDA_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/bitarray.h>
#include <commons/collections/queue.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
#include<math.h>
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
#define ID_COMANDA "ID_COMANDA"

t_config* config_comanda;
t_log* log_comanda;


t_list* hilos_clientes;
pthread_mutex_t hilos_clientes_mtx;

t_list* hilos_operaciones;
pthread_mutex_t hilos_operaciones_mtx;

t_list* paginas_swap;
pthread_mutex_t paginas_swap_mtx;

pthread_mutex_t tablas_paginas_mtx;

typedef struct{
	char* nombre;
	t_list* tabla_segmentos;
	pthread_mutex_t tabla_segmentos_mtx;
} t_restaurante;

typedef struct{
	int id_pedido;
	est_pedido estado;
	t_list* tabla_paginas;
} t_segmento;

typedef struct{
	void* mensaje;
	int socket_cliente;
} t_mensaje_a_procesar;

typedef struct{
	uint32_t frame;
	uint32_t pagina_swap;
	uint32_t ultimo_acceso; // time(NULL);
	bool presencia;
	bool uso; //se inicia en 1
	bool modificado; // se inicia en 0
}t_pagina;

typedef struct{
	uint32_t cant_pedida;
	uint32_t cant_lista;
	char nombre[24];
}t_plato;

typedef enum{
	CLOCK_MEJORADO = 0,
	LRU = 1,
}t_algoritmo_reemplazo;

t_algoritmo_reemplazo algoritmo_reemplazo;

pthread_mutex_t restaurantes_mtx;
t_list* restaurantes;

int cant_frames_swap;
int cant_frames_MP;
t_bitarray* frames_swap;
t_bitarray* frames_MP;

pthread_mutex_t frames_swap_mtx;
pthread_mutex_t frames_MP_mtx;

void* memoria_principal;
void* memoria_swap;

pthread_mutex_t memoria_principal_mtx;
pthread_mutex_t memoria_swap_mtx;

int puntero_clock;
pthread_mutex_t puntero_clock_mtx;


void iniciar_comanda();
void inicializar_swap();
void process_request(int cod_op, int cliente_fd);
void serve_client(int socket);
void esperar_cliente(int servidor);

t_restaurante* buscarRestaurante(char* nombre);
t_segmento* buscarPedido(uint32_t id_pedido, t_restaurante* restaurante);
void enviar_confirmacion(t_confirmacion _confirmacion, int cliente, op_code cod_op);
t_pagina* buscarPlato(t_list* tabla_paginas, char* comida);
void* serializar_pagina(t_plato* plato);

void ejecucion_guardar_pedido(t_mensaje_a_procesar* mensaje_a_procesar);
void ejecucion_guardar_plato(t_mensaje_a_procesar* mensaje_a_procesar);
void ejecucion_finalizar_pedido(t_mensaje_a_procesar* mensaje_a_procesar);
void ejecucion_confirmar_pedido(t_mensaje_a_procesar* mensaje_a_procesar);
void ejecucion_plato_listo(t_mensaje_a_procesar* mensaje_a_procesar);
void ejecucion_obtener_pedido(t_mensaje_a_procesar* mensaje_a_procesar);
void ejecucion_handshake_cliente(t_mensaje_a_procesar* mensaje_a_procesar);

int memoria_disponible_swap();
t_plato* deserializar_pagina(void* stream);
void guardar_en_swap(int frame_destino_swap, t_plato* plato);
int guardar_en_mp(t_plato* plato);
int seleccionar_frame_mp();
bool actualizar_plato_mp(t_pagina* pagina, int cantidad_pedida, int cantidad_lista);
int memoria_disponible_mp();
void traer_de_swap(t_pagina* pagina);
void actualizar_swap(t_pagina* pagina);

void liberar_pagina(t_pagina* pagina);
void free_pagina(t_pagina* pagina);

int eleccion_victima_clock_mejorado();
int eleccion_victima_LRU();
bool esta_en_MP(t_pagina* pagina);
void liberar_frame(t_pagina* victima);

void* list_iterate_and_find_from_index(t_list* self, void(closure)(void*), bool(*condition)(void*));
bool uso_modificado_cero(t_pagina* pagina);
bool uso_cero_modificado_uno(t_pagina* pagina);
void hacer_nada(void* algo); //jaja salu2
void cambiar_uso_cero(t_pagina* pagina);
int indice_siguiente(int i, int tope);

#endif /* UTILS_COMANDA_H_ */
