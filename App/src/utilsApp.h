#ifndef UTILSAPP_H_
#define UTILSAPP_H_

#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#include <commons/string.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>

#include "../../commonsCoronaLinux/socket.h"
#include "../../commonsCoronaLinux/logs.h"

// debug
    bool modo_noComanda;
    bool modo_noRest;

// logging
    t_log*  logger_obligatorio;
    bool    logger_obligatorio_consolaActiva;

    t_log*  logger_configuracion;
    bool    logger_configuracion_consolaActiva;
    char*   logger_configuracion_path;

    t_log*  logger_planificacion;
    bool    logger_planificacion_consolaActiva;
    char*   logger_planificacion_path;

    t_log*	logger_mensajes;
    bool    logger_mensajes_consolaActiva;
	char*   logger_mensajes_path;

// configuracion
    typedef enum {
        FIFO,
        HRRN,
        SJF_SD
    } t_algoritmoPlanificacion;

    void configuracionInicial(void);
    char* cfval_ipComanda;
    int cfval_puertoComanda;
    int cfval_puertoEscucha;
    int cfval_retardoCicloCPU;
    int cfval_gradoMultiprocesamiento;
    t_algoritmoPlanificacion cfval_algoritmoPlanificacion;
    double cfval_alpha;
    double cfval_estimacionInicial;
    char** cfval_platosDefault;
    int cfval_posicionRestDefaultX;
    int cfval_posicionRestDefaultY;

// restaurantes
    typedef struct {
        int pos_x;
        int pos_y;
        char* nombre;
    } t_restaurante;

    t_list* restaurantes;
    pthread_mutex_t mutex_lista_restaurantes;
    t_restaurante* get_restaurante(char* nombre_restaurante);

// clientes
    typedef struct {
        int pos_x;
        int pos_y;
        int id;
    } t_cliente;

    t_list* clientes;
    pthread_mutex_t mutex_lista_clientes;
    t_cliente* get_cliente(int id_cliente);

// repartidores
    typedef struct {
        unsigned id; // TODO: init
        int pos_x;
        int pos_y;
        int frecuenciaDescanso;
        int frecuenciaDescanso_restante;
        int tiempoDescanso;
        int tiempoDescanso_restante;
        bool tiene_pedido_asignado;
    } t_repartidor;

    t_list* repartidores;
    pthread_mutex_t mutex_lista_repartidores;
    sem_t semaforo_repartidoresSinPedido;
    bool repartidor_mover_hacia(t_repartidor* repartidor, int destino_x, int destino_y);
    void repartidor_disponibilizar(t_repartidor* repartidor);

// pedidos
    typedef enum {
        NEW,
        READY,
        EXEC,
        BLOCK,
        EXIT
    } t_estado;

    typedef struct {
        t_cliente* cliente;
        t_restaurante* restaurante;
        t_repartidor* repartidor;
        int pedido_id;
        t_estado pedido_estado;
        pthread_mutex_t* mutex_EXEC;
        pthread_t* hilo;
    } t_pedido;

    void* fhilo_pedido(void* pedido_sin_castear); // toma t_pedido* por param
    void consumir_ciclo(t_pedido* pedido);
    void pedido_repartidorLlegoARestaurante(t_pedido* pedido);
    void pedido_repartidorLlegoACliente(t_pedido* pedido);

// colas
    t_list* cola_NEW;
    pthread_mutex_t mutex_cola_NEW;
    void planif_encolar_NEW(t_pedido* pedido);

    t_list* cola_READY;
    pthread_mutex_t mutex_cola_READY;
    void planif_encolar_READY(t_pedido* pedido);

// planificadores
    void* fhilo_planificador_largoPlazo(void* __sin_uso__); // (de NEW a READY)
    void* fhilo_planificador_cortoPlazo(void* __sin_uso__); // (de READY a EXEC)
    void planif_nuevoPedido(int id_pedido, int id_cliente, char* nombre_restaurante);
    t_pedido* planif_asignarRepartidor(void);
    t_pedido* planif_FIFO(void);
    t_pedido* planif_SJF_SD(void);
    t_pedido* planif_HRRN(void);
    sem_t semaforo_pedidos_NEW;
    sem_t semaforo_pedidos_READY;
    sem_t semaforo_vacantesEXEC;
    unsigned distancia_entre(int ax, int ay, int bx, int by);

// liberacion de memoria
    void liberar_memoria(void);

/* CONEXIONES */

void configuracionConexiones(void);
t_list* hilos;
pthread_mutex_t mutex_hilos;
void* fhilo_conectarConComanda(void* arg);
void* fhilo_servidor(void* arg);
void esperar_cliente(int servidor);
void serve_client(int socket);
void process_request(int cod_op, int cliente_fd);
void conexionRecepcion(void);

sem_t sem_mensajes_a_enviar;
t_queue* mensajes_a_enviar;

pthread_t hilo_conectarConComanda;
pthread_t hilo_servidor;


#endif // UTILSAPP_H_
