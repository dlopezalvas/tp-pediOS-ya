#ifndef UTILSAPP_H_
#define UTILSAPP_H_

#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#include <commons/string.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>

// enums
    typedef enum {
        FIFO,
        HRRN,
        SJF_SD
    } t_algoritmoPlanificacion;

    typedef enum {
        NEW,
        READY,
        EXEC,
        BLOCK,
        EXIT
    } t_estado;

// structs
    typedef struct {
        char* nombreRestaurant;
        int idPedido;
        t_estado estadoPedido;
        t_repartidor* repartidor;
        pthread_mutex_t* mutex_EXEC;
        pthread_t* hilo;
    } t_pedido;

    typedef struct {
        int pos_x;
        int pos_y;
        int frecuenciaDescanso;
        int frecuenciaDescanso_restante; // TODO: inicalizar al asignar pedido
        int tiempoDescanso;
        int tiempoDescanso_restante; // TODO: inicalizar al asignar pedido
        pthread_mutex_t* mutex_asignarPedido;
    } t_repartidor;

    typedef struct {
        int pos_x;
        int pos_y;
        int id;
    } t_cliente;

// variables de logging
    t_log*  logger_obligatorio;
    bool    logger_obligatorio_consolaActiva;

    t_log*  logger_configuracion;
    bool    logger_configuracion_consolaActiva;
    char*   logger_configuracion_path;

// configuracion
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

// repartidores
    t_list* repartidores;
    pthread_mutex_t mutex_lista_repartidores;
    sem_t semaforo_repartidoresSinPedido;
    sem_t semaforo_vacantesEXEC;

// pedidos
    void* fhilo_pedido(void* pedido); // toma t_pedido* por param

// colas
    t_list* cola_NEW;
    pthread_mutex_t mutex_cola_NEW;
    void encolar_NEW(t_pedido* pedido);

    t_list* cola_READY;
    pthread_mutex_t mutex_cola_READY;
    void encolar_READY(t_pedido* pedido);

// planificadores
    void* fhilo_planificador_largoPlazo(void* __sin_uso__); // (de NEW a READY)
    void* fhilo_planificador_cortoPlazo(void* __sin_uso__); // (de READY a EXEC)

// liberacion de memoria
    void liberar_memoria(void);

#endif // UTILSAPP_H_