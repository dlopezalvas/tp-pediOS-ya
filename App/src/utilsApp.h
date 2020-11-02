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

// //TODO: eliminar cuando se actualicen las commons
// #define m_restaurante t_restaurante
// #define POSICION_RESTAURANTE 400
// #define RTA_POSICION_CLIENTE 401
// #define ERROR 402

extern int errno;

// debug
    bool modo_noComanda;
    bool modo_noRest;
    bool modo_mock;

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
    uint32_t cfval_id; // TODO: init
    t_restaurante_y_plato* platos_default_enviable;

// restaurantes
    typedef struct {
        pthread_mutex_t* mutex; // por ahora solo para el socket
        int socket;
        int pos_x;
        int pos_y;
        char* nombre;
        pthread_mutex_t* q_mtx; // TODO: init
        sem_t* q_sem; // TODO: init
        t_list* q; // TODO: init
        pthread_t* q_admin; // TODO: init
    } t_restaurante;

    typedef struct {
        t_mensaje* m_enviar;
        t_mensaje* m_recibir;
        pthread_mutex_t* mutex;
        error_flag_t* error_flag;
    } qr_form_t;

    t_list* restaurantes;
    pthread_mutex_t mutex_lista_restaurantes;
    unsigned resto_default_id_serial; // TODO: init
    unsigned resto_default_get_id(void);
    pthread_mutex_t resto_default_id_serial_mtx; // TODO: init
    t_restaurante* resto_default;
    t_restaurante* get_restaurante(char* nombre_restaurante);
    void guardar_nuevoRest(m_restaurante* mensaje_rest, int socket);
    t_list* get_nombresRestConectados(void);

    void qr_free_form(qr_form_t* form);
    qr_form_t* qr_request(t_mensaje* m_enviar, t_restaurante* rest);
    void* qr_admin(t_restaurante* rest);

// clientes
    typedef struct {
        pthread_mutex_t* mutex; // por ahora solo para el socket
        int pos_x;
        int pos_y;
        int id;
        int pedido_id;
        bool seleccionoPedido; // TODO: init en false obvio
        t_restaurante* restaurante_seleccionado;
        int socket;
    } t_cliente;

    t_list* clientes;
    pthread_mutex_t mutex_lista_clientes;
    // t_cliente* get_cliente(int id_pedido);
    t_cliente* get_cliente_porSuID(int id_cliente);
    void guardar_seleccion(char* nombre_rest, int id_cliente);

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
    void repartidor_desocupar(t_repartidor* repartidor);

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
        unsigned sjf_ultRafaga_real;
        double sjf_ultRafaga_est;
        unsigned hrrn_tiempoEsperaREADY;
        t_estado pedido_estado;
        pthread_mutex_t* estaPreparado; // TODO: init locked; lo unlockea el hilo que recibe el obtener pedido X/X
        pthread_mutex_t* mutex_EXEC;
        pthread_mutex_t* mutex_clock;
        pthread_t* hilo;
    } t_pedido;

    t_list* pedidos;
    t_list* pedidosEXEC;
    pthread_mutex_t mutex_pedidos;
    pthread_mutex_t mutex_pedidosEXEC;
    void* fhilo_pedido(void* pedido_sin_castear); // toma t_pedido* por param
    void consumir_ciclo(t_pedido* pedido);
    void pedido_repartidorLlegoARestaurante(t_pedido* pedido);
    void pedido_repartidorLlegoACliente(t_pedido* pedido);
    t_pedido* get_pedido(int id_pedido, char* nombre_restaurante, bool mutex_pedidos_locked_outside);

// colas
    t_list* cola_NEW;
    pthread_mutex_t mutex_cola_NEW;
    void planif_encolar_NEW(t_pedido* pedido);

    t_list* cola_READY;
    pthread_mutex_t mutex_cola_READY;
    void planif_encolar_READY(t_pedido* pedido);

    t_list* cola_BLOCK; // esta cola es para esperar los platos no terminados y para el descanso
    pthread_mutex_t mutex_cola_BLOCK;
    void planif_encolar_BLOCK(t_pedido* pedido);

// planificadores
    void* fhilo_planificador_largoPlazo(void* __sin_uso__); // (de NEW a READY)
    void* fhilo_planificador_cortoPlazo(void* __sin_uso__); // (de READY a EXEC)
    void* fhilo_clock(void* __sin_uso__);
    pthread_t hilo_planificador_cortoPlazo;
    pthread_t hilo_planificador_largoPlazo;
    pthread_t hilo_clock;
    void planif_nuevoPedido(int id_pedido);
    t_pedido* planif_asignarRepartidor(void);
    t_pedido* planif_FIFO(void);
    t_pedido* planif_SJF_SD(void);
    t_pedido* planif_HRRN(void);
    sem_t semaforo_pedidos_NEW;
    sem_t semaforo_pedidos_READY;
    sem_t semaforo_vacantesEXEC;
    double estimar_rafaga(t_pedido* pedido);
    double respRatio(t_pedido* pedido);
    bool SJF_o_HRRN(void);
    unsigned distancia_entre(int ax, int ay, int bx, int by);

// liberacion de memoria
    void liberar_memoria(void);

// otras utilidades
    void* search_remove_return(t_list* list, void* elem);

/* CONEXIONES */

void configuracionConexiones(void);
t_list* hilos;
pthread_mutex_t mutex_hilos;
void* fhilo_servidor(void* arg);
void esperar_cliente(int servidor);
void serve_client(int socket);
void process_request(int cod_op, int cliente_fd);
t_mensaje* mensajear_comanda(op_code cod_op_env, void* params, bool liberar_params);

sem_t sem_mensajes_a_enviar;
t_queue* mensajes_a_enviar;

pthread_t hilo_conectarConComanda;
pthread_t hilo_servidor;

void gestionar_POSICION_CLIENTE(int cliente_id, t_coordenadas* coord, int socket_cliente);
void gestionar_CONSULTAR_RESTAURANTES(int socket_cliente);
void gestionar_SELECCIONAR_RESTAURANTE(m_seleccionarRestaurante* seleccion, int socket_cliente);
void gestionar_CONSULTAR_PLATOS(int cliente_id, int socket_cliente);
void gestionar_CREAR_PEDIDO(int cliente_id, int socket_cliente);
void gestionar_AGREGAR_PLATO(t_nombre_y_id* plato, int cliente_id, int socket_cliente);
void gestionar_CONFIRMAR_PEDIDO(t_nombre_y_id* pedido, int socket_cliente, int cliente_id);
void gestionar_PLATO_LISTO(m_platoListo* plato);

bool todosLosPlatosEstanPreparados(rta_obtenerPedido* pedido);

#endif // UTILSAPP_H_
