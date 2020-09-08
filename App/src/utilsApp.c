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

    // lista de restaurantes
        restaurantes = list_create();
        pthread_mutex_init(&mutex_lista_restaurantes, NULL);

    // lista de clientes
        clientes = list_create();
        pthread_mutex_init(&mutex_lista_clientes, NULL);
}

void planif_encolar_NEW(t_pedido* pedido) {
    pthread_mutex_lock(&mutex_cola_NEW);
    pedido->pedido_estado = NEW;
    list_add(cola_NEW, pedido);
    pthread_mutex_unlock(&mutex_cola_NEW);
}

void planif_encolar_READY(t_pedido* pedido) {
    pthread_mutex_lock(&mutex_cola_READY);
    pedido->pedido_estado = READY;
    list_add(cola_READY, pedido);
    pthread_mutex_unlock(&mutex_cola_READY);
}

void* fhilo_planificador_largoPlazo(void* __sin_uso__) { // (de NEW a READY)
    t_pedido* pedido_seleccionado;
    while (1) {
        sem_wait(&semaforo_repartidoresSinPedido);
        sem_wait(&semaforo_pedidos_NEW);
        pthread_mutex_lock(&mutex_cola_NEW);
        pthread_mutex_lock(&mutex_lista_repartidores);
        pedido_seleccionado = planif_asignarRepartidor();
        pthread_mutex_unlock(&mutex_cola_NEW);
        pthread_mutex_unlock(&mutex_lista_repartidores);
        planif_encolar_READY(pedido_seleccionado);
    }
}

void* fhilo_planificador_cortoPlazo(void* __sin_uso__) { // (de READY a EXEC)
    t_pedido* pedido_seleccionado;
    while (1) {
        sem_wait(&semaforo_vacantesEXEC);
        sem_wait(&semaforo_pedidos_READY);
        pthread_mutex_lock(&mutex_cola_READY);
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
        pthread_mutex_unlock(&mutex_cola_READY);
        
        pthread_mutex_unlock(pedido_seleccionado->mutex_EXEC);
    }
}

t_pedido* planif_FIFO(void) {
    return list_remove(cola_READY, 0);
}

t_pedido* planif_SJF_SD(void) {
    // TODO
}

t_pedido* planif_HRRN(void) {
    // TODO
}

void planif_pedidoNuevo(int id_pedido, int id_cliente, char* nombre_restaurante) {
    t_restaurante* restaurante;
    t_cliente* cliente;
    t_pedido* pedidoNuevo;

    cliente = get_cliente(id_cliente);
    restaurante = get_restaurante(nombre_restaurante);

    pedidoNuevo = malloc(sizeof(t_pedido));
    pedidoNuevo->cliente = cliente;
    pedidoNuevo->restaurante = restaurante;
    pedidoNuevo->pedido_id = id_pedido;

    pedidoNuevo->mutex_EXEC = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(pedidoNuevo->mutex_EXEC, NULL);
    pthread_mutex_lock(pedidoNuevo->mutex_EXEC);

    pedidoNuevo->hilo = malloc(sizeof(pthread_t));
    pthread_create(pedidoNuevo->hilo, NULL, fhilo_pedido, pedidoNuevo);

    planif_encolar_NEW(pedidoNuevo);
}

t_cliente* get_cliente(int id_cliente) {
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

t_pedido* planif_asignarRepartidor(void) {
    // TODO
    // (no preocuparse por mutex)
    // repartidor->frecuenciaDescanso_restante = repartidor->frecuenciaDescanso;
    // repartidor->tiene_pedido_asignado = true;
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
    pedido_repartidorLlegoARestaurante(pedido);
    while (
        repartidor_mover_hacia(
            pedido->repartidor,
            pedido->cliente->pos_x,
            pedido->cliente->pos_y
        )
    ) {
        consumir_ciclo(pedido->repartidor);
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
    if (modo_noRest_noComanda) {
        return;
    }
    // TODO: casos reales lol
}

void pedido_repartidorLlegoACliente(t_pedido* pedido) {
    pedido->pedido_estado = EXIT;
    repartidor_disponibilizar(pedido->repartidor);
    // TODO: envio de mensaje al Cliente corresp.
}

void repartidor_disponibilizar(t_repartidor* repartidor) {
    repartidor->tiene_pedido_asignado = false;
    sem_post(&semaforo_repartidoresSinPedido);
}

void consumir_ciclo(t_pedido* pedido) {
    pedido->repartidor->frecuenciaDescanso_restante--;
    if (pedido->repartidor->frecuenciaDescanso_restante) {
        sleep(cfval_retardoCicloCPU);
        return;
    }
    pedido->pedido_estado = BLOCK;
    for (
        pedido->repartidor->tiempoDescanso_restante = pedido->repartidor->tiempoDescanso;
        pedido->repartidor->tiempoDescanso_restante > 0;
        pedido->repartidor->tiempoDescanso_restante--
    ) {
        sleep(cfval_retardoCicloCPU);
    }
    planif_encolar_READY(pedido);
    pthread_mutex_lock(pedido->mutex_EXEC);
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
