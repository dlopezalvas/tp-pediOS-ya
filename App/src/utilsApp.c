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
        logger_planificacion = log_create(logger_planificacion_path, program_name, logger_planificacion_consolaActiva, LOG_LEVEL_DEBUG);

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
            repartidor->id = index_repartidores;
            
            log_debug(logger_configuracion, "[CONFIG] |\t|\tPos. X: %i", atoi(string_split(posicionesRepartidores[index_repartidores], ",")[0])); //TODO: ojo separators para cuando corrijan el enunciado
            repartidor->pos_x = atoi(string_split(posicionesRepartidores[index_repartidores], ",")[0]); //TODO: ojo separators para cuando corrijan el enunciado
            
            log_debug(logger_configuracion, "[CONFIG] |\t|\tPos. Y: %i", atoi(string_split(posicionesRepartidores[index_repartidores], ",")[1])); //TODO: ojo separators para cuando corrijan el enunciado
            repartidor->pos_y = atoi(string_split(posicionesRepartidores[index_repartidores], ",")[1]); //TODO: ojo separators para cuando corrijan el enunciado
            
            log_debug(logger_configuracion, "[CONFIG] |\t|\tTiempo de descanso: %i", atoi(tiemposDescanso[index_repartidores]));
            repartidor->tiempoDescanso = atoi(tiemposDescanso[index_repartidores]);
            
            log_debug(logger_configuracion, "[CONFIG] |\t|\tFrecuencia de descanso: %i", atoi(frecuenciasDescanso[index_repartidores]));
            repartidor->frecuenciaDescanso = atoi(frecuenciasDescanso[index_repartidores]);

            repartidor->tiene_pedido_asignado = false;

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
        sem_init(&semaforo_pedidos_NEW, 0, 0);

        cola_READY = list_create();
        pthread_mutex_init(&mutex_cola_READY, NULL);
        sem_init(&semaforo_pedidos_READY, 0, 0);

        cola_BLOCK = list_create();
        pthread_mutex_init(&mutex_cola_BLOCK, NULL);

        pedidosEXEC = list_create();
        pthread_mutex_init(&mutex_pedidosEXEC, NULL);

    // lista de pedidos
        pedidos = list_create();
        pthread_mutex_init(&mutex_pedidos, NULL);

    // lista de restaurantes
        restaurantes = list_create();
        pthread_mutex_init(&mutex_lista_restaurantes, NULL);

    // lista de clientes
        clientes = list_create();
        pthread_mutex_init(&mutex_lista_clientes, NULL);
}

void planif_encolar_NEW(t_pedido* pedido) {
    pthread_mutex_lock(&mutex_cola_NEW);
    log_debug(logger_planificacion, "[PLANIF_NP] Encolando pedido %i en NEW...", pedido->pedido_id);
    pedido->pedido_estado = NEW;
    list_add(cola_NEW, pedido);
    pthread_mutex_unlock(&mutex_cola_NEW);
    sem_post(&semaforo_pedidos_NEW);
}

void planif_encolar_READY(t_pedido* pedido) {
    pthread_mutex_lock(&mutex_cola_READY);
    switch (pedido->pedido_estado) {
        case NEW:
            log_debug(logger_planificacion, "[PLANIF_LP] Encolando pedido %i en READY...", pedido->pedido_id);
            break;
        case BLOCK:
            log_debug(logger_planificacion, "[REP_%2i] Encolando pedido %i en READY...", pedido->repartidor->id, pedido->pedido_id);
    }
    pedido->pedido_estado = READY;
    list_add(cola_READY, pedido);
    pthread_mutex_unlock(&mutex_cola_READY);
    sem_post(&semaforo_pedidos_READY);
}

void planif_encolar_BLOCK(t_pedido* pedido) {
    pthread_mutex_lock(&mutex_pedidosEXEC);
    pthread_mutex_lock(&mutex_cola_BLOCK);
    log_debug(logger_planificacion, "[PEDIDO_%2i] Desencolando de EXEC...", pedido->pedido_id);
    search_remove_return(pedidosEXEC, pedido);
    pedido->pedido_estado = BLOCK;
    log_debug(logger_planificacion, "[PEDIDO_%2i] Encolando en BLOCK...", pedido->pedido_id);
    list_add(cola_BLOCK, pedido);
    pthread_mutex_unlock(&mutex_pedidosEXEC);
    pthread_mutex_unlock(&mutex_cola_BLOCK);
    sem_post(&semaforo_vacantesEXEC);
}

void* fhilo_planificador_largoPlazo(void* __sin_uso__) { // (de NEW a READY)
    t_pedido* pedido_seleccionado;
    log_debug(logger_planificacion, "[PLANIF_LP] Hilo comenzando...");
    while (1) {
        log_debug(logger_planificacion, "[PLANIF_LP] Esperando repartidor disponible");
        sem_wait(&semaforo_repartidoresSinPedido);

        log_debug(logger_planificacion, "[PLANIF_LP] Hay repartidor(es) disponible(s; esperando pedido en NEW");
        sem_wait(&semaforo_pedidos_NEW);

        log_debug(logger_planificacion, "[PLANIF_LP] Hay pedido(s) en NEW; esperando para lockear cola de NEW");
        pthread_mutex_lock(&mutex_cola_NEW);

        log_debug(logger_planificacion, "[PLANIF_LP] Cola de NEW lockeada; esperando para lockear lista de repartidores");
        pthread_mutex_lock(&mutex_lista_repartidores);

        log_debug(logger_planificacion, "[PLANIF_LP] Lista de repartidores lockeada");

        pedido_seleccionado = planif_asignarRepartidor();

        log_debug(logger_planificacion, "[PLANIF_LP] Unlockeando cola de NEW...");
        pthread_mutex_unlock(&mutex_cola_NEW);

        log_debug(logger_planificacion, "[PLANIF_LP] Unlockeando lista de repartidores...");
        pthread_mutex_unlock(&mutex_lista_repartidores);

        planif_encolar_READY(pedido_seleccionado);
    }
}

void* fhilo_planificador_cortoPlazo(void* __sin_uso__) { // (de READY a EXEC)
    t_pedido* pedido_seleccionado;
    log_debug(logger_planificacion, "[PLANIF_CP] Hilo comenzando...");
    while (1) {
        log_debug(logger_planificacion, "[PLANIF_CP] Esperando vacante para EXEC");
        sem_wait(&semaforo_vacantesEXEC);

        log_debug(logger_planificacion, "[PLANIF_CP] Hay vacante(s) para EXEC; esperando pedidos en READY");
        sem_wait(&semaforo_pedidos_READY);

        log_debug(logger_planificacion, "[PLANIF_CP] Hay pedido(s) en READY; esperando para lockear cola de READY");
        pthread_mutex_lock(&mutex_cola_READY);

        log_debug(logger_planificacion, "[PLANIF_CP] Cola de READY lockeada");

        switch (cfval_algoritmoPlanificacion) {
            case FIFO:
                pedido_seleccionado = planif_FIFO();
                break;
            case SJF_SD:
                pedido_seleccionado = planif_SJF_SD();
                break;
            case HRRN:
                pedido_seleccionado = planif_HRRN();
        }

        log_debug(logger_planificacion, "[PLANIF_CP] Pedido seleccionado: %i", pedido_seleccionado->pedido_id);

        log_debug(logger_planificacion, "[PLANIF_CP] Unlockeando cola de READY...");
        pthread_mutex_unlock(&mutex_cola_READY);

        // TODO: logging
        pthread_mutex_lock(&mutex_pedidosEXEC);
        pedido_seleccionado->pedido_estado = EXEC;
        list_add(pedidosEXEC, pedido_seleccionado);
        pthread_mutex_unlock(&mutex_pedidosEXEC);
        
        log_debug(logger_planificacion, "[PLANIF_CP] Unlockeando EXEC de pedido %i", pedido_seleccionado->pedido_id);
        pthread_mutex_unlock(pedido_seleccionado->mutex_EXEC);

        log_debug(logger_planificacion, "[PLANIF_CP] Recomenzando ciclo...");
    }
}

t_pedido* planif_FIFO(void) {
    log_debug(logger_planificacion, "[PLANIF_CP] Desencolando pedido de READY...");
    return list_remove(cola_READY, 0);
}

t_pedido* planif_SJF_SD(void) {
    // TODO
}

t_pedido* planif_HRRN(void) {
    // TODO
}

void planif_nuevoPedido(int id_pedido) {
    t_restaurante* restaurante;
    t_cliente* cliente;
    t_pedido* pedidoNuevo;

    log_debug(logger_planificacion, "[PLANIF_NP] Planificando nuevo pedido: %i", id_pedido);

    cliente = get_cliente(id_pedido);
    log_debug(logger_planificacion, "[PLANIF_NP] Cliente: %i", cliente->id);

    restaurante = cliente->restaurante_seleccionado; // TODO
    log_debug(logger_planificacion, "[PLANIF_NP] Restaurante: %s", restaurante->nombre);

    pedidoNuevo = malloc(sizeof(t_pedido));
    pedidoNuevo->cliente = cliente;
    pedidoNuevo->restaurante = restaurante;
    pedidoNuevo->pedido_id = id_pedido;

    pedidoNuevo->mutex_clock = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(pedidoNuevo->mutex_clock, NULL);
    pthread_mutex_lock(pedidoNuevo->mutex_clock); // TODO: init lockeado?
    log_debug(logger_planificacion, "[PLANIF_NP] Mutex de clock inicializado");

    pedidoNuevo->mutex_EXEC = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(pedidoNuevo->mutex_EXEC, NULL);
    pthread_mutex_lock(pedidoNuevo->mutex_EXEC);
    log_debug(logger_planificacion, "[PLANIF_NP] Mutex de EXEC inicializado");

    pedidoNuevo->hilo = malloc(sizeof(pthread_t));
    log_debug(logger_planificacion, "[PLANIF_NP] Lanzando hilo del pedido...");
    pthread_create(pedidoNuevo->hilo, NULL, fhilo_pedido, pedidoNuevo);

    planif_encolar_NEW(pedidoNuevo);
}

t_cliente* get_cliente_porSuID(int id_cliente) {
    t_cliente* cliente;
    pthread_mutex_lock(&mutex_lista_clientes);
    for (
        unsigned index_cli = 0;
        index_cli < list_size(clientes);
        index_cli++
    ) {
        cliente = list_get(clientes, index_cli);
        if (cliente->id == id_cliente) {
            pthread_mutex_unlock(&mutex_lista_clientes);
            return cliente;
        }
    }
    pthread_mutex_unlock(&mutex_lista_clientes);
    return NULL;
}

t_cliente* get_cliente(int id_pedido) {
    t_cliente* cliente;
    pthread_mutex_lock(&mutex_lista_clientes);
    for (
        unsigned index_cli = 0;
        index_cli < list_size(clientes);
        index_cli++
    ) {
        cliente = list_get(clientes, index_cli);
        if (cliente->pedido_id == id_pedido) {
            pthread_mutex_unlock(&mutex_lista_clientes);
            return cliente;
        }
    }
    pthread_mutex_unlock(&mutex_lista_clientes);
    return NULL;
}

t_restaurante* get_restaurante(char* nombre_restaurante) {
    t_restaurante* restaurante;
    pthread_mutex_lock(&mutex_lista_restaurantes);
    for (
        unsigned index_rest = 0;
        index_rest < list_size(restaurantes);
        index_rest++
    ) {
        restaurante = list_get(restaurantes, index_rest);
        if (
            string_equals_ignore_case(
                nombre_restaurante,
                restaurante->nombre
            )
        ) {
            pthread_mutex_unlock(&mutex_lista_restaurantes);
            return restaurante;        
        }
    }
    pthread_mutex_unlock(&mutex_lista_restaurantes);
    return NULL;
}

t_pedido* planif_asignarRepartidor(void) { // TODO: logging
    t_list* repartidores_disponibles;
    t_pedido* pedido;
    t_repartidor* repartidor;
    t_pedido* pedido_a_planif;
    t_repartidor* repartidor_a_planif;
    unsigned distancia;
    unsigned distancia_minima;

    repartidores_disponibles = list_create();

    for (
        unsigned index_repartidor = 0;
        index_repartidor < list_size(repartidores);
        index_repartidor++
    ) {
        repartidor = list_get(repartidores, index_repartidor);
        if (!(repartidor->tiene_pedido_asignado))
            list_add(repartidores_disponibles, repartidor);
    }

    if (list_is_empty(repartidores_disponibles))
        log_debug(logger_planificacion, "[PLANIF_LP] Algo horrible ha sucedido");

    repartidor = NULL;
    distancia_minima = -1;

    for (
        unsigned index_pedido = 0;
        index_pedido < list_size(cola_NEW);
        index_pedido++
    ) {
        pedido = list_get(cola_NEW, index_pedido);
        for (
            unsigned index_repartidor = 0;
            index_repartidor < list_size(repartidores_disponibles);
            index_repartidor++
        ) {
            repartidor = list_get(repartidores_disponibles, index_repartidor);
            distancia = distancia_entre(
                            repartidor->pos_x,
                            repartidor->pos_y,
                            pedido->restaurante->pos_x,
                            pedido->restaurante->pos_y
                        );
            if (distancia < distancia_minima) {
                distancia_minima = distancia;
                repartidor_a_planif = repartidor;
                pedido_a_planif = pedido;
            }
        }
    }

    pedido_a_planif->repartidor = repartidor_a_planif;
    repartidor_a_planif->tiene_pedido_asignado = true;
    repartidor_a_planif->frecuenciaDescanso_restante = repartidor_a_planif->frecuenciaDescanso;
    return pedido_a_planif;
}

unsigned distancia_entre(int ax, int ay, int bx, int by) {
    return abs(ax - bx) + abs(ay - by);
}

void* fhilo_pedido(void* pedido_sin_castear) { // toma t_pedido* por param
    t_pedido* pedido = (t_pedido*) pedido_sin_castear;
    log_debug(logger_planificacion, "[PEDIDO_%2i] Hilo comenzando...", pedido->pedido_id);
    log_debug(logger_planificacion, "[PEDIDO_%2i] Lanzando hilo del pedido...", pedido->pedido_id);
    pthread_mutex_lock(pedido->mutex_EXEC);
    while (
        repartidor_mover_hacia(
            pedido->repartidor,
            pedido->restaurante->pos_x,
            pedido->restaurante->pos_y
        )
    ) {
        consumir_ciclo(pedido);
    }
    pedido_repartidorLlegoARestaurante(pedido);
    while (
        repartidor_mover_hacia(
            pedido->repartidor,
            pedido->cliente->pos_x,
            pedido->cliente->pos_y
        )
    ) {
        consumir_ciclo(pedido);
    }
    pedido_repartidorLlegoACliente(pedido);
    // TODO: que pasa con el pedido luego de finalizar?
}

// mover_hacia() mueve el repartidor un casillero hacia el destino
//      > si se realiza un movimiento, devuelve true
//      > si no se realiza un movimiento (porque llego a destino), devuelve false 
bool repartidor_mover_hacia(t_repartidor* repartidor, int destino_x, int destino_y) {
    // TODO: logging
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
    if (modo_noRest) {
        log_debug(
            logger_planificacion,
            "[PEDIDO_%2i] Llego a restaurante %s",
            pedido->pedido_id,
            pedido->restaurante->nombre
        );
        return;
    }
    // TODO: casos reales lol
}

void pedido_repartidorLlegoACliente(t_pedido* pedido) {
    pedido->pedido_estado = EXIT;
    log_debug(
        logger_planificacion,
        "[PEDIDO_%2i] Llegado a cliente %2i; pasado a EXIT",
        pedido->pedido_id,
        pedido->cliente->id
    );
    repartidor_desocupar(pedido->repartidor);
    // TODO: envio de mensaje al cliente corresp.
    t_mensaje* mensaje = malloc(sizeof(t_mensaje));
    mensaje->tipo_mensaje = FINALIZAR_PEDIDO;
    // TODO
}

void repartidor_desocupar(t_repartidor* repartidor) {
    log_debug(
        logger_planificacion,
        "[REP_%2i] Disponibilizado",
        repartidor->id
    );
    repartidor->tiene_pedido_asignado = false;
    sem_post(&semaforo_repartidoresSinPedido);
}

void consumir_ciclo(t_pedido* pedido) {
    pedido->repartidor->frecuenciaDescanso_restante--;
    log_debug(
        logger_planificacion,
        "[PEDIDO_%2i] Movido a posicion (%i;%i)",
        pedido->pedido_id,
        pedido->repartidor->pos_x,
        pedido->repartidor->pos_y
    );
    log_debug(
        logger_planificacion,
        "[PEDIDO_%2i] Ciclo consumido (restan %i)",
        pedido->pedido_id,
        pedido->repartidor->frecuenciaDescanso_restante
    );
    if (pedido->repartidor->frecuenciaDescanso_restante) {
        // sleep(cfval_retardoCicloCPU);
        pthread_mutex_lock(pedido->mutex_clock);
        return;
    }
    log_debug(
        logger_planificacion,
        "[PEDIDO_%2i] Bloqueando por descanso...",
        pedido->pedido_id
    );
    planif_encolar_BLOCK(pedido);
    for (
        pedido->repartidor->tiempoDescanso_restante = pedido->repartidor->tiempoDescanso;
        pedido->repartidor->tiempoDescanso_restante > 0;
        pedido->repartidor->tiempoDescanso_restante--
    ) {
        // sleep(cfval_retardoCicloCPU);
        pthread_mutex_lock(pedido->mutex_clock);
        log_debug(
        logger_planificacion,
        "[PEDIDO_%2i] Ciclo descanso consumido (restan %i)",
        pedido->pedido_id,
        pedido->repartidor->tiempoDescanso_restante
    );
    }
    log_debug(
        logger_planificacion,
        "[PEDIDO_%2i] Descanso terminado",
        pedido->pedido_id
    );
    pedido->repartidor->frecuenciaDescanso_restante = pedido->repartidor->frecuenciaDescanso;
    planif_encolar_READY(pedido);
    log_debug(
        logger_planificacion,
        "[PEDIDO_%2i] Esperando unlock EXEC",
        pedido->pedido_id
    );
    pthread_mutex_lock(pedido->mutex_EXEC);
}

void guardar_nuevoRest(m_restaurante* mensaje_rest, int socket) { // TODO: commons
    t_restaurante* restaurante = malloc(sizeof(t_restaurante));
    restaurante->nombre = mensaje_rest->nombre;
    restaurante->pos_x = mensaje_rest->pos_x;
    restaurante->pos_y = mensaje_rest->pos_y;
    restaurante->socket = socket;
    restaurante->mutex = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(restaurante->mutex, NULL);
    pthread_mutex_lock(&mutex_lista_restaurantes);
    list_add(restaurantes, restaurante);
    // TODO: logging
    pthread_mutex_unlock(&mutex_lista_restaurantes);
}

void guardar_seleccion(char* nombre_rest, int id_cliente) {
    // TODO: logging
    t_restaurante* restaurante_seleccion;
    t_cliente* cliente_seleccion;

    pthread_mutex_lock(&mutex_lista_restaurantes);
    for (
        unsigned index_rest = 0;
        index_rest < list_size(restaurantes);
        index_rest++
    ) {
        restaurante_seleccion = list_get(restaurantes, index_rest);
        if (
            string_equals_ignore_case(
                nombre_rest,
                restaurante_seleccion->nombre
            )
        ) break;
    }
    pthread_mutex_unlock(&mutex_lista_restaurantes);
    pthread_mutex_lock(&mutex_lista_clientes);
    for (
        unsigned index_cli = 0;
        index_cli < list_size(clientes);
        index_cli++
    ) {
        cliente_seleccion = list_get(clientes, index_cli);
        if (cliente_seleccion->id == id_cliente) {
            cliente_seleccion->restaurante_seleccionado = restaurante_seleccion;
        }
    }
    pthread_mutex_unlock(&mutex_lista_clientes);
}

t_list* get_nombresRestConectados(void) {
    t_list* nombresRestConectados;
    t_nombre* restaurante;
    nombresRestConectados = list_create();
    pthread_mutex_lock(&mutex_lista_restaurantes);
    if (list_is_empty(restaurantes) || modo_noRest) {
        pthread_mutex_unlock(&mutex_lista_restaurantes);
        restaurante = malloc(sizeof(t_nombre));
        restaurante->nombre = "Resto Default";
        return nombresRestConectados;
    }
    for (
        unsigned index_rest = 0;
        index_rest < list_size(restaurantes);
        index_rest++
    ) {
        restaurante = malloc(sizeof(t_nombre));
        restaurante->nombre = string_duplicate(((t_restaurante*)list_get(restaurantes, index_rest))->nombre);
        list_add(nombresRestConectados, restaurante);
    }
    pthread_mutex_unlock(&mutex_lista_restaurantes);
    return nombresRestConectados;
}

void* fhilo_clock(void* __sin_uso__) {
    while (true) {
        pthread_mutex_lock(&mutex_cola_BLOCK);
        pthread_mutex_lock(&mutex_pedidosEXEC);
        for (
            unsigned index_EXEC = 0;
            index_EXEC < list_size(pedidosEXEC);
            index_EXEC++
        ) {
            pthread_mutex_unlock(((t_pedido*)list_get(pedidosEXEC, index_EXEC))->mutex_clock);
        }
        for (
            unsigned index_BLOCK = 0;
            index_BLOCK < list_size(cola_BLOCK);
            index_BLOCK++
        ) {
            pthread_mutex_unlock(((t_pedido*)list_get(cola_BLOCK, index_BLOCK))->mutex_clock);
        }
        pthread_mutex_unlock(&mutex_cola_BLOCK);
        pthread_mutex_unlock(&mutex_pedidosEXEC);
        sleep(cfval_retardoCicloCPU);
    }
}

void* search_remove_return(t_list* list, void* elem) {
    for (
        unsigned index_elem = 0;
        index_elem < list_size(list);
        index_elem++
    ) {
        if (elem == list_get(list, index_elem)) {
            return list_remove(list, index_elem);
        }
    }
    return NULL;
}

void liberar_memoria(void) {
    // TODO jajaj
}


































/* CONEXIONES */

void configuracionConexiones(void) {
	logger_mensajes = log_create(logger_mensajes_path, "App", logger_mensajes_consolaActiva, LOG_LEVEL_DEBUG);
    hilos = list_create();
}


// para enviar un mensaje
    // queue_push(mensajes_a_enviar, mensaje);
    // sem_post(&sem_mensajes_a_enviar);
void* fhilo_conectarConComanda(void* arg) {
    // TODO: corregir todo esto
    int socket = iniciar_cliente(cfval_ipComanda, cfval_puertoComanda);

	if(socket != -1){

		while(1){ //buscar condicion de que siga ejecutando
			sem_wait(&sem_mensajes_a_enviar);
			t_mensaje* mensaje = queue_pop(mensajes_a_enviar);
			enviar_mensaje(mensaje, socket);
			loggear_mensaje_enviado(mensaje->parametros, mensaje->tipo_mensaje, logger_mensajes);
//			free_struct_mensaje(mensaje->parametros, mensaje->tipo_mensaje);
		}
    }
}

void* fhilo_servidor(void* arg) {
    int conexion_servidor;
    conexion_servidor = iniciar_servidor(cfval_puertoEscucha);
    
    while(1) {
        esperar_cliente(conexion_servidor);
    }
}

void esperar_cliente(int servidor){
	struct sockaddr_in direccion_cliente;

	unsigned int tam_direccion = sizeof(struct sockaddr_in);

	int cliente = accept (servidor, (void*) &direccion_cliente, &tam_direccion);
	pthread_t hilo;

	pthread_mutex_lock(&mutex_hilos);
	list_add(hilos, &hilo);
	pthread_mutex_unlock(&mutex_hilos);

	pthread_create(&hilo,NULL,(void*)serve_client,cliente);
	pthread_detach(hilo);

}

void serve_client(int socket){
	int rec;
	int cod_op;
	while(1){
		rec = recv(socket, &cod_op, sizeof(op_code), MSG_WAITALL);
		if(rec == -1 || rec == 0 ){
			cod_op = -1;
			//			pthread_mutex_lock(&logger_mutex);
			//			log_info(logger,"Se desconecto el proceso con id: %d",socket);
			//			pthread_mutex_unlock(&logger_mutex);
			pthread_exit(NULL);
		}
		puts("recibi un mensaje");
		printf("codigo: %d\n", cod_op);
		process_request(cod_op, socket);
	}
}

void process_request(int cod_op, int cliente_fd) {
    int size = 0;
    void* mensaje = NULL;
    int rec;
    uint32_t cliente_id;

    // TODO: provisorio?
    rec = recv(socket, &cliente_id, sizeof(uint32_t), MSG_WAITALL);
    if(rec == -1 || rec == 0 ){
        cod_op = -1;
        //			pthread_mutex_lock(&logger_mutex);
        //			log_info(logger,"Se desconecto el proceso con id: %d",socket);
        //			pthread_mutex_unlock(&logger_mutex);
        pthread_exit(NULL);
    }

    if(op_code_to_struct_code(cod_op) != STRC_MENSAJE_VACIO){
        void* buffer = recibir_mensaje(cliente_fd, &size);
        mensaje = deserializar_mensaje(buffer, cod_op);
    }

    // TODO loggear_mensaje_recibido(mensaje_deserializado, cod_op, logger_mensajes);

    // TODO: switch con tipos de mensaje
    switch (cod_op) {

        // de CLIENTE:
            case POSICION_CLIENTE:
                gestionar_POSICION_CLIENTE(mensaje, cliente_fd);
                    // OK definicion
                    // OK implementacion
                break;
            case CONSULTAR_RESTAURANTES:
                gestionar_CONSULTAR_RESTAURANTES(cliente_fd);
                    // OK definicion
                    // OK implementacion
                break;
            case SELECCIONAR_RESTAURANTE:
                gestionar_SELECCIONAR_RESTAURANTE(mensaje, cliente_fd);
                    // OK definicion
                    // OK implementacion
                break;
            case CONSULTAR_PLATOS:
                gestionar_CONSULTAR_PLATOS(cliente_id, cliente_fd);
                    // OK definicion
                    // OK implementacion
                break;
            case CREAR_PEDIDO:
                gestionar_CREAR_PEDIDO(cliente_id, cliente_fd);
                    // OK definicion
                    // TODO implementacion
                break;
            case AGREGAR_PLATO:
                gestionar_AGREGAR_PLATO(mensaje, cliente_id, cliente_fd);
                    // OK definicion
                    // TODO implementacion
                break;
            case CONFIRMAR_PEDIDO:
                gestionar_CONFIRMAR_PEDIDO(mensaje, cliente_fd);
                    // OK definicion
                    // TODO implementacion
                break;

        // de RESTAURANT:
            case POSICION_RESTAURANTE: // TODO commons
                guardar_nuevoRest(mensaje, cliente_fd);
                    // OK definicion
                    // TODO implementacion
                break;

            case PLATO_LISTO:
                gestionar_PLATO_LISTO(mensaje);
                    // OK definicion
                    // TODO implementacion
                break;

        // TODO: liberar mensaje aca o dentro de cada f?
    }
}


void conexionRecepcion(){
	int socket_servidor = iniciar_cliente(cfval_ipComanda, cfval_puertoComanda);

	int size = 0;
	op_code cod_op;
	int _recv;
	while (1) {
		_recv = recv(socket_servidor, &cod_op, sizeof(op_code), MSG_WAITALL);

		if (_recv != -1 && _recv != 0) {
			void* buffer = recibir_mensaje(socket_servidor, &size);
			void* mensaje = deserializar_mensaje(buffer, cod_op);
            // TODO: switch para mensajes comanda
			loggear_mensaje_recibido(mensaje, cod_op, logger_mensajes);
			free(buffer);
		}
        // else {
		// 	loggear_mensaje_recibido(NULL, cod_op, logger_mensajes);
		// }
	}

	liberar_conexion(socket_servidor);
}

void gestionar_POSICION_CLIENTE(m_cliente* datos_cliente, int socket_cliente) {
    t_cliente* cliente = malloc(sizeof(t_cliente));
    t_mensaje* mensaje = malloc(sizeof(t_mensaje));
    uint32_t* confirmacion = malloc(sizeof(uint32_t));

    cliente->id = datos_cliente->id;
    cliente->pos_x = datos_cliente->posicion.x;
    cliente->pos_y = datos_cliente->posicion.y;
    cliente->restaurante_seleccionado = NULL;
    cliente->socket = socket_cliente;
    cliente->mutex = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(cliente->mutex, NULL);
    pthread_mutex_lock(cliente->mutex);

    pthread_mutex_lock(&mutex_lista_clientes);
    list_add(clientes, cliente);
    // TODO: logging
    pthread_mutex_unlock(&mutex_lista_clientes);

    mensaje->tipo_mensaje = RTA_POSICION_CLIENTE;
    *confirmacion = 1;
    mensaje->parametros = confirmacion;
    enviar_mensaje(mensaje, socket_cliente);

    pthread_mutex_unlock(cliente->mutex);
    free_struct_mensaje(mensaje->parametros, mensaje->tipo_mensaje);
    free(mensaje);
}

void gestionar_CONSULTAR_RESTAURANTES(int socket_cliente) {
    t_mensaje* mensaje = malloc(sizeof(t_mensaje));
    t_restaurante_y_plato* restaurantes = malloc(sizeof(t_restaurante_y_plato));

    mensaje->tipo_mensaje = RTA_CONSULTAR_RESTAURANTES;
    restaurantes->nombres = get_nombresRestConectados(); // TODO: redefinir a t_list*
    restaurantes->cantElementos = list_size(restaurantes->nombres);
    mensaje->parametros = restaurantes;
    enviar_mensaje(mensaje, socket_cliente);
    free_struct_mensaje(mensaje->parametros, mensaje->tipo_mensaje);
    free(mensaje);
}

void gestionar_SELECCIONAR_RESTAURANTE(m_seleccionarRestaurante* seleccion, int socket_cliente) {
    t_mensaje* mensaje = malloc(sizeof(t_mensaje));
    uint32_t* confirmacion = malloc(sizeof(uint32_t));
    t_cliente* cliente_seleccionante;

    cliente_seleccionante = get_cliente_porSuID(seleccion->cliente);
    cliente_seleccionante->restaurante_seleccionado = get_restaurante(seleccion->restaurante.nombre);

    mensaje->tipo_mensaje = RTA_SELECCIONAR_RESTAURANTE;
    *confirmacion = 1;
    mensaje->parametros = confirmacion;
    enviar_mensaje(mensaje, socket_cliente);
    free_struct_mensaje(mensaje->parametros, mensaje->tipo_mensaje);
    free(mensaje);
}

void gestionar_CONSULTAR_PLATOS(int cliente_id, int socket_cliente) {
    t_mensaje* mensaje = malloc(sizeof(t_mensaje));
    t_nombre* nombre_rest_consultado = malloc(sizeof(t_nombre));
    t_restaurante* restaurante_consultado;
    t_cliente* cliente_consultor;
    op_code cod_op;
    int id_recibida;
    int _recv_op;
    int _recv_id;
    int size = 0;

    cliente_consultor = get_cliente_porSuID(cliente_id);
    restaurante_consultado = cliente_consultor->restaurante_seleccionado; // TODO: error handling si no selecciono restaurante

    mensaje->tipo_mensaje = CONSULTAR_PLATOS;
    nombre_rest_consultado->nombre="";
    nombre_rest_consultado->largo_nombre=0;
    mensaje->parametros = nombre_rest_consultado;

    pthread_mutex_lock(restaurante_consultado->mutex);
    enviar_mensaje(mensaje, restaurante_consultado->socket);
    free_struct_mensaje(mensaje->parametros, mensaje->tipo_mensaje);
    free(mensaje);

    _recv_op = recv(restaurante_consultado->socket, &cod_op, sizeof(op_code), MSG_WAITALL);
    if (_recv_op != -1 && _recv_op != 0) {
        // TODO: error handling
    }

    _recv_id = recv(restaurante_consultado->socket, &id_recibida, sizeof(op_code), MSG_WAITALL);
    if (_recv_id != -1 && _recv_id != 0) {
        // TODO: error handling
    }

    switch (cod_op) {
        case ERROR:
            // TODO: error handling
            break;
        case RTA_CONSULTAR_PLATOS:
            mensaje = malloc(sizeof(t_mensaje));
            mensaje->tipo_mensaje = cod_op;
            mensaje->parametros = deserializar_mensaje(
                recibir_mensaje(restaurante_consultado->socket, &size),
                cod_op
            );
            enviar_mensaje(mensaje, socket_cliente);
            free_struct_mensaje(mensaje->parametros, mensaje->tipo_mensaje);
            free(mensaje);
            break;
    }
}

void gestionar_CREAR_PEDIDO(int cliente_id, int socket_cliente) {
    // buscar rest de ese cliente
    // mandarle el crear pedido
    // esperar respuesta
    // mandarle guardar pedido a comanda
    // esperar respuesta
    // responder a cliente ok
}

void gestionar_AGREGAR_PLATO(t_nombre_y_id* plato, int cliente_id, int socket_cliente) {
    // buscar rest de ese cliente
    // mandarle el agregar plato
    // esperar respuesta
    // mandarle guardar plato a comanda
    // esperar respuesta
    // responder a cliente ok
}

void gestionar_CONFIRMAR_PEDIDO(t_nombre_y_id* pedido, int socket_cliente) {
    
}

void gestionar_PLATO_LISTO(m_platoListo* plato) {

}