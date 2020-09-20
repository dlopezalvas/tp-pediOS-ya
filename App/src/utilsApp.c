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
    pthread_mutex_lock(&mutex_cola_BLOCK);
    log_debug(logger_planificacion, "[PEDIDO_%2i] Encolando en BLOCK...", pedido->pedido_id);
    pedido->pedido_estado = BLOCK;
    list_add(cola_BLOCK, pedido);
    pthread_mutex_unlock(&mutex_cola_BLOCK);
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

    pedidoNuevo->mutex_EXEC = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(pedidoNuevo->mutex_EXEC, NULL);
    pthread_mutex_lock(pedidoNuevo->mutex_EXEC);
    log_debug(logger_planificacion, "[PLANIF_NP] Mutex de EXEC inicializado");

    pedidoNuevo->hilo = malloc(sizeof(pthread_t));
    log_debug(logger_planificacion, "[PLANIF_NP] Lanzando hilo del pedido...");
    pthread_create(pedidoNuevo->hilo, NULL, fhilo_pedido, pedidoNuevo);

    planif_encolar_NEW(pedidoNuevo);
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
        if (repartidor->tiene_pedido_asignado)
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
        sleep(cfval_retardoCicloCPU);
        return;
    }
    log_debug(
        logger_planificacion,
        "[PEDIDO_%2i] Bloqueando por descanso...",
        pedido->pedido_id
    );
    pedido->pedido_estado = BLOCK;
    sem_post(&semaforo_vacantesEXEC);
    for (
        pedido->repartidor->tiempoDescanso_restante = pedido->repartidor->tiempoDescanso;
        pedido->repartidor->tiempoDescanso_restante > 0;
        pedido->repartidor->tiempoDescanso_restante--
    ) {
        sleep(cfval_retardoCicloCPU);
    }
    log_debug(
        logger_planificacion,
        "[PEDIDO_%2i] Descanso terminado",
        pedido->pedido_id
    );
    planif_encolar_READY(pedido);
    log_debug(
        logger_planificacion,
        "[PEDIDO_%2i] Unlockeando EXEC",
        pedido->pedido_id
    );
    pthread_mutex_lock(pedido->mutex_EXEC);
}

void guardar_nuevoCliente(int id, int pos_x, int pos_y) {
    t_cliente* cliente = malloc(sizeof(t_cliente));
    cliente->id = id;
    cliente->pos_x = pos_x;
    cliente->pos_y = pos_y;
    cliente->restaurante_seleccionado = NULL;
    pthread_mutex_lock(&mutex_lista_clientes);
    list_add(clientes, cliente);
    // TODO: logging
    pthread_mutex_unlock(&mutex_lista_clientes);
}


void guardar_nuevoRest(char* nombre, int pos_x, int pos_y) {
    t_restaurante* restaurante = malloc(sizeof(t_restaurante));
    restaurante->nombre = nombre;
    restaurante->pos_x = pos_x;
    restaurante->pos_y = pos_y;
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

char** get_nombresRestConectados(void) {
    char** nombresRestConectados;
    pthread_mutex_lock(&mutex_lista_restaurantes);
    if (list_is_empty(restaurantes) || modo_noRest) {
        pthread_mutex_unlock(&mutex_lista_restaurantes);
        nombresRestConectados = malloc(sizeof(char*));
        *nombresRestConectados = "Resto Default";
        return nombresRestConectados;
    }
    nombresRestConectados = malloc(sizeof(char*) * list_size(restaurantes));
    for (
        unsigned index_rest = 0;
        index_rest < list_size(restaurantes);
        index_rest++
    ) {
        nombresRestConectados[index_rest] = string_duplicate(((t_restaurante*)list_get(restaurantes, index_rest))->nombre);
    }
    pthread_mutex_unlock(&mutex_lista_restaurantes);
    return nombresRestConectados;
}

void liberar_memoria(void) {
    // TODO
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
	void* buffer = recibir_mensaje(cliente_fd, &size);
	
    void* mensaje_deserializado = deserializar_mensaje(buffer, cod_op);
    loggear_mensaje_recibido(mensaje_deserializado, cod_op, logger_mensajes);

    // TODO: switch con tipos de mensaje
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
