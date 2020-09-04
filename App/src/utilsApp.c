#include "utilsApp.h"

// configuracionInicial() inicializa:
//      > los loggers
//      > todos los cfval_* (config values)
//      > la lista de repartidores
//      > el semaforo de repartidores libres
//      > el semaforo de vacantes para EXEC
//      > colas NEW y READY con sus mutexes
void configuracionInicial(void) {
    // auxiliares
        char** posicionesRepartidores;
        char** frecuenciasDescanso;
        char** tiemposDescanso;
        char* pathArchivoLog;
        char* algoritmoPlanificacion;
        t_repartidor* repartidor;

    // para los loggers
        char* program_name = "App";

    // config local, todos los valores finales se guardan en variables globales
        t_config* config = config_create("App.config");

    // configuracion de loggers adicionales para debug
        logger_configuracion = log_create(logger_configuracion_path, program_name, logger_configuracion_consolaActiva, LOG_LEVEL_DEBUG);

    // configuracion del logger para el log obligatorio
        pathArchivoLog = config_get_string_value(config, "ARCHIVO_LOG");
        log_debug(logger_configuracion, "[CONFIG] Path del log obligatorio: %s", pathArchivoLog);
        
        logger_obligatorio = log_create(pathArchivoLog, program_name, logger_obligatorio_consolaActiva, LOG_LEVEL_INFO);

    // configuracion de ip y puertos
        cfval_ipComanda = config_get_string_value(config, "IP_COMANDA");
        log_debug(logger_configuracion, "[CONFIG] IP de Comanda: %s", cfval_ipComanda);

        cfval_puertoComanda = config_get_int_value(config, "PUERTO_COMANDA");
        log_debug(logger_configuracion, "[CONFIG] Puerto de Comanda: %i", cfval_puertoComanda);

        cfval_puertoEscucha = config_get_int_value(config, "PUERTO_ESCUCHA");
        log_debug(logger_configuracion, "[CONFIG] Puerto de escucha: %i", cfval_puertoEscucha);

    // configuracion del procesador
        cfval_retardoCicloCPU = config_get_int_value(config, "RETARDO_CICLO_CPU");
        log_debug(logger_configuracion, "[CONFIG] Retardo de ciclo de CPU: %i", cfval_retardoCicloCPU);
        
        cfval_gradoMultiprocesamiento = config_get_int_value(config, "GRADO_DE_MULTIPROCESAMIENTO");
        log_debug(logger_configuracion, "[CONFIG] Grado de multiprocesamiento: %i", cfval_gradoMultiprocesamiento);
        sem_init(&semaforo_vacantesEXEC, 0, cfval_gradoMultiprocesamiento);

    // configuracion del algoritmo de planificacion
        algoritmoPlanificacion = config_get_string_value(config, "ALGORITMO_DE_PLANIFICACION");
        log_debug(logger_configuracion, "[CONFIG] Algoritmo de planificacion: %s", algoritmoPlanificacion);
        
        if (string_equals_ignore_case("FIFO", algoritmoPlanificacion))
            cfval_algoritmoPlanificacion = FIFO;

        if (string_equals_ignore_case("HRRN", algoritmoPlanificacion))
            cfval_algoritmoPlanificacion = HRRN;

        // se lee alpha y estimacion inicial solo si es SJF
        if (string_equals_ignore_case("SJF-SD", algoritmoPlanificacion)) {
            cfval_algoritmoPlanificacion = SJF_SD;
            
            cfval_alpha = config_get_double_value(config, "ALPHA");
            log_debug(logger_configuracion, "[CONFIG] Alpha: %i", cfval_alpha);
            
            cfval_estimacionInicial = config_get_double_value(config, "ESTIMACION_INICIAL");
            log_debug(logger_configuracion, "[CONFIG] Estimacion inicial: %i", cfval_estimacionInicial);
        }

    // configuracion de repartidores
        log_debug(logger_configuracion, "[CONFIG] Repartidores:");
        
        posicionesRepartidores = string_split(config_get_string_value(config, "REPARTIDORES"), "|"); //TODO: ojo separators para cuando corrijan el enunciado
        frecuenciasDescanso = string_split(config_get_string_value(config, "FRECUENCIA_DE_DESCANSO"), "|"); //TODO: ojo separators para cuando corrijan el enunciado
        tiemposDescanso = string_split(config_get_string_value(config, "TIEMPO_DE_DESCANSO"), "|"); //TODO: ojo separators para cuando corrijan el enunciado
        
        repartidores = list_create();

        for (
            unsigned index_repartidores = 0;
            posicionesRepartidores[index_repartidores];
            index_repartidores++
        ) {
            log_debug(logger_configuracion, "[CONFIG] |\t<%i>", index_repartidores);
            repartidor = malloc(sizeof(t_repartidor));
            
            log_debug(logger_configuracion, "[CONFIG] |\t|\tPos. X: %i", atoi(string_split(posicionesRepartidores[index_repartidores], ",")[0])); //TODO: ojo separators para cuando corrijan el enunciado
            repartidor->pos_x = atoi(string_split(posicionesRepartidores[index_repartidores], ",")[0]); //TODO: ojo separators para cuando corrijan el enunciado
            
            log_debug(logger_configuracion, "[CONFIG] |\t|\tPos. Y: %i", atoi(string_split(posicionesRepartidores[index_repartidores], ",")[1])); //TODO: ojo separators para cuando corrijan el enunciado
            repartidor->pos_y = atoi(string_split(posicionesRepartidores[index_repartidores], ",")[1]); //TODO: ojo separators para cuando corrijan el enunciado
            
            log_debug(logger_configuracion, "[CONFIG] |\t|\tTiempo de descanso: %i", atoi(tiemposDescanso[index_repartidores]));
            repartidor->tiempoDescanso = atoi(tiemposDescanso[index_repartidores]);
            
            log_debug(logger_configuracion, "[CONFIG] |\t|\tFrecuencia de descanso: %i", atoi(frecuenciasDescanso[index_repartidores]));
            repartidor->frecuenciaDescanso = atoi(frecuenciasDescanso[index_repartidores]);

            repartidor->mutex_asignarPedido = malloc(sizeof(pthread_mutex_t));
            pthread_mutex_init(repartidor->mutex_asignarPedido, NULL);
            pthread_mutex_lock(repartidor->mutex_asignarPedido);

            list_add(repartidores, repartidor);
        }
        
        pthread_mutex_init(&mutex_lista_repartidores, NULL);
        sem_init(&semaforo_repartidoresSinPedido, 0, list_size(repartidores));

    // configuracion del restaurant default
        cfval_platosDefault = string_split(config_get_string_value(config, "PLATOS_DEFAULT"), "|"); //TODO: ojo separators para cuando corrijan el enunciado
        log_debug(logger_configuracion, "[CONFIG] Platos del restaurant default:");
        
        for (
            unsigned index_platos = 0;
            cfval_platosDefault[index_platos];
            index_platos++
        ) {
            log_debug(logger_configuracion, "[CONFIG] |\t%s", cfval_platosDefault[index_platos]);
        }
        
        cfval_posicionRestDefaultX = config_get_int_value(config, "POSICION_REST_DEFAULT_X");
        log_debug(logger_configuracion, "[CONFIG] Pos. X restaurant default: %i", cfval_posicionRestDefaultX);
        
        cfval_posicionRestDefaultY = config_get_int_value(config, "POSICION_REST_DEFAULT_Y");
        log_debug(logger_configuracion, "[CONFIG] Pos. Y restaurant default: %i", cfval_posicionRestDefaultY);

    // inicializacion de colas
        cola_NEW = list_create();
        pthread_mutex_init(&mutex_cola_NEW, NULL);

        cola_READY = list_create();
        pthread_mutex_init(&mutex_cola_READY, NULL);
}

void encolar_NEW(t_pedido* pedido) {
    pthread_mutex_lock(&mutex_cola_NEW);
    list_add(cola_NEW, pedido);
    pthread_mutex_unlock(&mutex_cola_NEW);
}

void encolar_READY(t_pedido* pedido) {
    pthread_mutex_lock(&mutex_cola_READY);
    list_add(cola_READY, pedido);
    pthread_mutex_unlock(&mutex_cola_READY);
}

void* fhilo_planificador_largoPlazo(void* __sin_uso__) { // (de NEW a READY)
    // TODO
}

void* fhilo_planificador_cortoPlazo(void* __sin_uso__) { // (de READY a EXEC)
    // TODO
}

void planificarPedidoNuevo(int id_pedido, int id_cliente, char* nombre_restaurante) {
    // TODO
}

void* fhilo_pedido(void* pedido_sin_castear) { // toma t_pedido* por param
    t_pedido* pedido = (t_pedido*) pedido_sin_castear;
    pthread_mutex_lock(pedido->mutex_EXEC); // ???
    while (
        repartidor_mover_hacia(
            pedido->repartidor,
            pedido->restaurante->pos_x,
            pedido->restaurante->pos_y
        )
    ) {
        consumir_ciclo(pedido->repartidor);
    }
    pedido_repartidorLlegoARestaurante(pedido); // TODO: los ciclos solo se consumen en movimiento?
    while (
        repartidor_mover_hacia(
            pedido->repartidor,
            pedido->cliente->pos_x,
            pedido->cliente->pos_y
        )
    ) {
        consumir_ciclo(pedido->repartidor);
    }
    pedido_repartidorLlegoACliente(pedido); // TODO: los ciclos solo se consumen en movimiento?
    // TODO: que pasa con el pedido luego de finalizar?
}

// mover_hacia() mueve el repartidor un casillero hacia el destino
//      > si se realiza un movimiento, devuelve true
//      > si no se realiza un movimiento (porque llego a destino), devuelve false 
bool repartidor_mover_hacia(t_repartidor* repartidor, int destino_x, int destino_y) {
    if (repartidor->pos_x > destino_x) {
        repartidor->pos_x--;
        return true;
    }
    if (repartidor->pos_x < destino_x) {
        repartidor->pos_x++;
        return true;
    }
    if (repartidor->pos_y > destino_y) {
        repartidor->pos_y--;
        return true;
    }
    if (repartidor->pos_y < destino_y) {
        repartidor->pos_y++;
        return true;
    }
    return false;
}

void pedido_repartidorLlegoARestaurante(t_pedido* pedido) {
    // TODO
}

void pedido_repartidorLlegoACliente(t_pedido* pedido) {
    // TODO
}

void consumir_ciclo(t_repartidor* repartidor) {
    // TODO   
}

void liberar_memoria(void) {
    // TODO
        // t_log*  logger_obligatorio;
        // t_log*  logger_configuracion;
        // t_list* repartidores;
        // t_list* cola_NEW;
        // t_list* cola_READY;
}