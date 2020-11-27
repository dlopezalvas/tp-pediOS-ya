#include "utilsApp.h"
#include <errno.h>

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
	t_nombre* nombre_plato_default;

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

	// configuracion del id de app
	cfval_id = config_get_int_value(config, "ID_APP");
	log_debug(logger_configuracion, "[CONFIG] ID de App: %i", cfval_id);

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

	posicionesRepartidores = config_get_array_value(config, "REPARTIDORES"); //TODO: ojo separators para cuando corrijan el enunciado
	frecuenciasDescanso = config_get_array_value(config, "FRECUENCIA_DE_DESCANSO"); //TODO: ojo separators para cuando corrijan el enunciado
	tiemposDescanso = config_get_array_value(config, "TIEMPO_DE_DESCANSO"); //TODO: ojo separators para cuando corrijan el enunciado

	repartidores = list_create();

	int index_repartidores = 0;

	char** posicion_repartidor;
	while(posicionesRepartidores[index_repartidores] != NULL){

		log_debug(logger_configuracion, "[CONFIG] |\t<%i>", index_repartidores);
		repartidor = malloc(sizeof(t_repartidor));
		repartidor->id = index_repartidores;

		posicion_repartidor = string_split(posicionesRepartidores[index_repartidores], "|");

		repartidor->pos_x = atoi(posicion_repartidor[0]);
		log_debug(logger_configuracion, "[CONFIG] |\t|\tPos. X: %i", repartidor->pos_x, ","); //TODO: ojo separators para cuando corrijan el enunciado

		repartidor->pos_y = atoi(posicion_repartidor[1]);
		log_debug(logger_configuracion, "[CONFIG] |\t|\tPos. Y: %i", repartidor->pos_y, ",");

		repartidor->tiempoDescanso = atoi(tiemposDescanso[index_repartidores]);
		log_debug(logger_configuracion, "[CONFIG] |\t|\tTiempo de descanso: %i", repartidor->tiempoDescanso);

		repartidor->frecuenciaDescanso = atoi(frecuenciasDescanso[index_repartidores]);
		log_debug(logger_configuracion, "[CONFIG] |\t|\tFrecuencia de descanso: %i", repartidor->frecuenciaDescanso);

		repartidor->tiene_pedido_asignado = false;

		list_add(repartidores, repartidor);

		liberar_vector(posicion_repartidor);

		index_repartidores++;
	}

	liberar_vector(posicionesRepartidores);
	liberar_vector(frecuenciasDescanso);
	liberar_vector(tiemposDescanso);


	pthread_mutex_init(&mutex_lista_repartidores, NULL);
	sem_init(&semaforo_repartidoresSinPedido, 0, list_size(repartidores));

	// configuracion del restaurant default
	cfval_platosDefault = config_get_array_value(config, "PLATOS_DEFAULT"); //TODO: ojo separators para cuando corrijan el enunciado
	log_debug(logger_configuracion, "[CONFIG] Platos del restaurant default:");

	platos_default_enviable = malloc(sizeof(t_restaurante_y_plato));
	platos_default_enviable->nombres = list_create();

	for (
			unsigned index_platos = 0;
			cfval_platosDefault[index_platos];
			index_platos++
	) {
		nombre_plato_default = malloc(sizeof(t_nombre));
		nombre_plato_default->nombre = cfval_platosDefault[index_platos];
		list_add(platos_default_enviable->nombres, nombre_plato_default);
		log_debug(logger_configuracion, "[CONFIG] |\t%s", cfval_platosDefault[index_platos]);
	}

	cfval_posicionRestDefaultX = config_get_int_value(config, "POSICION_REST_DEFAULT_X");
	log_debug(logger_configuracion, "[CONFIG] Pos. X restaurant default: %i", cfval_posicionRestDefaultX);

	cfval_posicionRestDefaultY = config_get_int_value(config, "POSICION_REST_DEFAULT_Y");
	log_debug(logger_configuracion, "[CONFIG] Pos. Y restaurant default: %i", cfval_posicionRestDefaultY);

	resto_default = malloc(sizeof(t_restaurante));
	resto_default->nombre = "RestoDefault";
	resto_default->pos_x = cfval_posicionRestDefaultX;
	resto_default->pos_y = cfval_posicionRestDefaultY;

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

	// liberacion de memoria
	// config_destroy(config);
}

void planif_encolar_NEW(t_pedido* pedido) {
	pthread_mutex_lock(&mutex_cola_NEW);
	log_debug(logger_planificacion, "[PLANIF_NP] Encolando pedido %i (%s) en NEW...", pedido->pedido_id, pedido->restaurante->nombre);
	pedido->pedido_estado = NEW;
	list_add(cola_NEW, pedido);
	pthread_mutex_unlock(&mutex_cola_NEW);
	sem_post(&semaforo_pedidos_NEW);
}

void planif_encolar_READY(t_pedido* pedido) {
	pthread_mutex_lock(&mutex_cola_READY);
	switch (pedido->pedido_estado) {
	case NEW:
		log_debug(logger_planificacion, "[PLANIF_LP] Encolando pedido %i (rep. %i) en READY...", pedido->pedido_id, pedido->repartidor->id);
		break;
	case BLOCK:
		log_debug(logger_planificacion, "[R%i-P%i] Encolando  en READY...", pedido->repartidor->id, pedido->pedido_id);
	}
	pedido->pedido_estado = READY;
	list_add(cola_READY, pedido);
	log_debug(logger_planificacion, "[R%i-P%i] Pedido encolado en READY", pedido->repartidor->id, pedido->pedido_id);
	pthread_mutex_unlock(&mutex_cola_READY);
	sem_post(&semaforo_pedidos_READY);
}

void planif_encolar_BLOCK(t_pedido* pedido) {
	pthread_mutex_lock(&mutex_cola_BLOCK);
	pthread_mutex_lock(&mutex_pedidosEXEC);
	log_debug(logger_planificacion, "[R%i-P%i] Desencolando de EXEC...", pedido->repartidor->id, pedido->pedido_id);
	search_remove_return(pedidosEXEC, pedido);
	pedido->pedido_estado = BLOCK;
	log_debug(logger_planificacion, "[R%i-P%i] Encolando en BLOCK...", pedido->repartidor->id, pedido->pedido_id);
	list_add(cola_BLOCK, pedido);
	pthread_mutex_unlock(&mutex_cola_BLOCK);
	pthread_mutex_unlock(&mutex_pedidosEXEC);
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

		if (SJF_o_HRRN()) {
			pedido_seleccionado->sjf_ultRafaga_est = estimar_rafaga(pedido_seleccionado);
			pedido_seleccionado->sjf_ultRafaga_real = 0;
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

bool SJF_o_HRRN(void) {
	return ((cfval_algoritmoPlanificacion == SJF_SD) || (cfval_algoritmoPlanificacion == HRRN));
}

t_pedido* planif_FIFO(void) {
	log_debug(logger_planificacion, "[PLANIF_CP] Desencolando pedido de READY...");
	return list_remove(cola_READY, 0);
}

t_pedido* planif_SJF_SD(void) {
	unsigned index_pedido_min = 0;
	double estimacion_min = estimar_rafaga(list_get(cola_READY, index_pedido_min));
	for (
			unsigned index_READY = 1;
			index_READY < list_size(cola_READY);
			index_READY++
	) {
		if (estimar_rafaga(list_get(cola_READY, index_READY)) < estimacion_min) {
			index_pedido_min = index_READY;
		}
	}
	return list_remove(cola_READY, index_pedido_min);
}

double estimar_rafaga(t_pedido* pedido) {
	// Tn+1 = ta + (1-a)Tn ... de donde sale estimacion inicial?
	return (pedido->sjf_ultRafaga_real) * cfval_alpha
			+ (pedido->sjf_ultRafaga_est) * (1 - cfval_alpha);
}

t_pedido* planif_HRRN(void) {
	unsigned index_pedido_max = 0;
	double respRatio_max = estimar_rafaga(list_get(cola_READY, index_pedido_max));
	for (
			unsigned index_READY = 1;
			index_READY < list_size(cola_READY);
			index_READY++
	) {
		if (respRatio(list_get(cola_READY, index_READY)) > respRatio_max) {
			index_pedido_max = index_READY;
		}
	}
	return list_remove(cola_READY, index_pedido_max);
}

double respRatio(t_pedido* pedido) {
	// Tn+1 = ta + (1-a)Tn ... de donde sale estimacion inicial?
	return 1 + (pedido->hrrn_tiempoEsperaREADY) / estimar_rafaga(pedido);
}

void planif_nuevoPedido(int id_cliente, int pedido_id) { // TODO: que devuelva bool si algo anda mal
	t_restaurante* restaurante;
	t_cliente* cliente;
	t_pedido* pedidoNuevo;

	log_debug(logger_planificacion, "[PLANIF_NP] Planificando nuevo pedido");

	cliente = get_cliente_porSuID(id_cliente);
	log_debug(logger_planificacion, "[PLANIF_NP] Puntero al cliente: %x", cliente);

	if (!cliente) {
		// TODO: error handling
		log_debug(logger_planificacion, "[PLANIF_NP] No existe puntero al cliente %i", id_cliente);
		log_debug(logger_planificacion, "[PLANIF_NP] Aca hay que retornar ERROR pero no esta hecho todavia");
		return;
	}
	log_debug(logger_planificacion, "[PLANIF_NP] Cliente: %i", cliente->id);

	restaurante = cliente->restaurante_seleccionado; // TODO
	if (!restaurante) {
		// TODO: error handling, ya esta hecho en otras partes! abstraer
	}
	log_debug(logger_planificacion, "[PLANIF_NP] Restaurante: %s", restaurante->nombre);

	pedidoNuevo = malloc(sizeof(t_pedido));
	pedidoNuevo->cliente = cliente;
	pedidoNuevo->restaurante = restaurante;
	pedidoNuevo->pedido_id = pedido_id;

	pedidoNuevo->sjf_ultRafaga_est = cfval_estimacionInicial;
	pedidoNuevo->sjf_ultRafaga_real = cfval_estimacionInicial;
	pedidoNuevo->hrrn_tiempoEsperaREADY = 0;

	pedidoNuevo->mutex_clock = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(pedidoNuevo->mutex_clock, NULL);
	pthread_mutex_lock(pedidoNuevo->mutex_clock); // TODO: init lockeado? porque despues medio que por defecto anda deslockeado D:
	log_debug(logger_planificacion, "[PLANIF_NP] Mutex de clock inicializado");

	pedidoNuevo->estaPreparado = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(pedidoNuevo->estaPreparado, NULL);
	pthread_mutex_lock(pedidoNuevo->estaPreparado);
	log_debug(logger_planificacion, "[PLANIF_NP] Mutex de 'esta preparado' inicializado");

	pedidoNuevo->mutex_EXEC = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(pedidoNuevo->mutex_EXEC, NULL);
	pthread_mutex_lock(pedidoNuevo->mutex_EXEC);
	log_debug(logger_planificacion, "[PLANIF_NP] Mutex de EXEC inicializado");

	pedidoNuevo->hilo = malloc(sizeof(pthread_t));
	log_debug(logger_planificacion, "[PLANIF_NP] Lanzando hilo del pedido...");
	pthread_create(pedidoNuevo->hilo, NULL, fhilo_pedido, pedidoNuevo);

	pthread_mutex_lock(&mutex_pedidos);
	list_add(pedidos, pedidoNuevo);
	pthread_mutex_unlock(&mutex_pedidos);

	planif_encolar_NEW(pedidoNuevo);
}

t_cliente* get_cliente_porSuID(int id_cliente) {
	t_cliente* cliente;
	log_debug(logger_planificacion, "[??????] Lockeando mutex_lista_clientes...");
	pthread_mutex_lock(&mutex_lista_clientes);
	for (
			unsigned index_cli = 0;
			index_cli < list_size(clientes);
			index_cli++
	) {
		log_debug(logger_planificacion, "[??????] Cliente de indice %i...", index_cli);
		cliente = list_get(clientes, index_cli);
		if (cliente->id == id_cliente) {
			log_debug(logger_planificacion, "[??????] \tEncontrado!");
			pthread_mutex_unlock(&mutex_lista_clientes);
			return cliente;
		}
	}
	log_debug(logger_planificacion, "[??????] No se encontro cliente, se retorna NULL");
	pthread_mutex_unlock(&mutex_lista_clientes);
	return NULL;
}

// t_cliente* get_cliente(int id_pedido) { // TODO: redo esto, el id no es univoco
//     t_cliente* cliente;
//     pthread_mutex_lock(&mutex_lista_clientes);
//     for (
//         unsigned index_cli = 0;
//         index_cli < list_size(clientes);
//         index_cli++
//     ) {
//         cliente = list_get(clientes, index_cli);
//         if (cliente->pedido_id == id_pedido) {
//             pthread_mutex_unlock(&mutex_lista_clientes);
//             return cliente;
//         }
//     }
//     pthread_mutex_unlock(&mutex_lista_clientes);
//     return NULL;
// }

t_restaurante* get_restaurante(char* nombre_restaurante) {
	t_restaurante* restaurante;
	if (string_equals_ignore_case(
			nombre_restaurante,
			resto_default->nombre
	)) {
		return resto_default;
	}
	pthread_mutex_lock(&mutex_lista_restaurantes);
	if (list_is_empty(restaurantes)) {
		return resto_default;
	}
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
	unsigned
	index_pedido_a_planif,
	index_repartidor_a_planif;
	unsigned distancia;
	unsigned distancia_minima;

	repartidores_disponibles = list_create(); // todo: liberar lista al final (pero no los elementos!)

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

	// log_debug(logger_planificacion, "[PLANIF_LP] D. minima: %u", distancia_minima);
	// log_debug(logger_planificacion, "[PLANIF_LP] Cant. elems. en cola NEW: %i", list_size(cola_NEW));
	// log_debug(logger_planificacion, "[PLANIF_LP] Cant. elems. en rep. disp.: %i", list_size(repartidores_disponibles));

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
				index_repartidor_a_planif = index_repartidor;
				index_pedido_a_planif = index_pedido;
			}
		}
	}
	// log_debug(logger_planificacion, "[PLANIF_LP] index_repartidor_a_planif = %i", index_repartidor_a_planif);
	// log_debug(logger_planificacion, "[PLANIF_LP] index_pedido_a_planif = %i", index_pedido_a_planif);
	pedido_a_planif = list_remove(cola_NEW, index_pedido_a_planif);
	// log_debug(logger_planificacion, "[PLANIF_LP] list remove 1");
	pedido_a_planif->repartidor = list_remove(repartidores_disponibles, index_repartidor_a_planif);
	// log_debug(logger_planificacion, "[PLANIF_LP] list remove 2");
	pedido_a_planif->repartidor->tiene_pedido_asignado = true;
	pedido_a_planif->repartidor->frecuenciaDescanso_restante = pedido_a_planif->repartidor->frecuenciaDescanso;
	log_debug(
			logger_planificacion,
			"[PLANIF_LP] Se asigno el rep. %i al pedido %i (%s) del cliente %i",
			pedido_a_planif->repartidor->id,
			pedido_a_planif->pedido_id,
			pedido_a_planif->restaurante->nombre,
			pedido_a_planif->cliente->id
	);
	list_destroy(repartidores_disponibles);
	return pedido_a_planif;
}

unsigned distancia_entre(int ax, int ay, int bx, int by) {
	return abs(ax - bx) + abs(ay - by);
}

void* fhilo_pedido(void* pedido_sin_castear) { // toma t_pedido* por param
	t_pedido* pedido = (t_pedido*) pedido_sin_castear;
	log_debug(logger_planificacion, "[R?-P%i] Hilo comenzando...", pedido->pedido_id);
	log_debug(logger_planificacion, "[R?-P%i] Lanzando hilo del pedido...", pedido->pedido_id);
	// pthread_mutex_trylock(pedido->mutex_EXEC);
	pthread_mutex_lock(pedido->mutex_EXEC);
	log_debug(logger_planificacion, "[R?-P%i] Post lock EXEC", pedido->pedido_id);
	pthread_mutex_trylock(pedido->mutex_clock);
	log_debug(logger_planificacion, "[R?-P%i] Post trylock clock", pedido->pedido_id);
	pthread_mutex_lock(pedido->mutex_clock);
	log_debug(logger_planificacion, "[R?-P%i] Post lock clock", pedido->pedido_id);
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
	log_debug(
			logger_planificacion,
			"[R%i-P%i] Llego a restaurante %s",
			pedido->repartidor->id,
			pedido->pedido_id,
			pedido->restaurante->nombre
	);
	if (modo_noRest) { return; }
	if (pedido->restaurante != resto_default) {
		bool noEstaPreparado = pthread_mutex_trylock(pedido->estaPreparado);
		if (noEstaPreparado) {
			log_debug(
					logger_planificacion,
					"[R%i-P%i] El pedido no esta preparado todavia",
					pedido->repartidor->id,
					pedido->pedido_id
			);

			planif_encolar_BLOCK(pedido);
			pthread_mutex_lock(pedido->estaPreparado);

			pthread_mutex_lock(&mutex_cola_BLOCK);
			search_remove_return(cola_BLOCK, pedido);
			pthread_mutex_unlock(&mutex_cola_BLOCK);

			planif_encolar_READY(pedido);
			pthread_mutex_lock(pedido->mutex_EXEC);
		}
	}
	log_debug(
			logger_planificacion,
			"[R%i-P%i] Saliendo del restaurante %s hacia el cliente %i",
			pedido->repartidor->id,
			pedido->pedido_id,
			pedido->restaurante->nombre,
			pedido->cliente->id
	);
	return;
}

void pedido_repartidorLlegoACliente(t_pedido* pedido) {
	t_nombre_y_id params_FP;
	t_mensaje* rta_comanda;
	t_mensaje* mje_env_cli;

	pthread_mutex_lock(&mutex_pedidosEXEC);
	// pthread_mutex_lock(&mutex_pedidosEXIT);
	search_remove_return(pedidosEXEC, pedido);
	pedido->pedido_estado = EXIT;
	pthread_mutex_unlock(&mutex_pedidosEXEC);
	sem_post(&semaforo_vacantesEXEC);
	// pthread_mutex_unlock(&mutex_pedidosEXIT);
	log_debug(
			logger_planificacion,
			"\n************************************************************************************************************************************\n**************************************** [R%i-P%i] Llegado a cliente %i; pasado a EXIT\n************************************************************************************************************************************",
			pedido->repartidor->id,
			pedido->pedido_id,
			pedido->cliente->id
	);
	repartidor_desocupar(pedido->repartidor);

	if (modo_mock) return;

	params_FP.id = pedido->pedido_id;
	params_FP.nombre.nombre = pedido->restaurante->nombre;
	rta_comanda = mensajear_comanda(FINALIZAR_PEDIDO, &params_FP, false);
	if (!rta_comanda) {
		// TODO ???
	}
	switch (rta_comanda->tipo_mensaje) {
	case ERROR: // TODO
		free_struct_mensaje(rta_comanda->parametros, rta_comanda->tipo_mensaje);
		free(rta_comanda);
		break;
		// return;
	case RTA_OBTENER_PEDIDO:
		free_struct_mensaje(rta_comanda->parametros, rta_comanda->tipo_mensaje);
		free(rta_comanda);
		break;
	default: // TODO
		free_struct_mensaje(rta_comanda->parametros, rta_comanda->tipo_mensaje);
		free(rta_comanda);
		// return;
	}
	mje_env_cli = malloc(sizeof(t_mensaje));
	mje_env_cli->tipo_mensaje = FINALIZAR_PEDIDO;
	mje_env_cli->id = cfval_id;
	mje_env_cli->parametros = &params_FP;
	enviar_mensaje(mje_env_cli, pedido->cliente->socket); // TODO: mutex socket
	free(mje_env_cli);
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
	pedido->sjf_ultRafaga_real++;
	log_debug(
			logger_planificacion,
			"[R%i-P%i] Movido a posicion (%i,%i); ciclo consumido (restan %i)",
			pedido->repartidor->id,
			pedido->pedido_id,
			pedido->repartidor->pos_x,
			pedido->repartidor->pos_y,
			pedido->repartidor->frecuenciaDescanso_restante
	);
	if (pedido->repartidor->frecuenciaDescanso_restante) {
		// sleep(cfval_retardoCicloCPU);
		pthread_mutex_trylock(pedido->mutex_clock);
		pthread_mutex_lock(pedido->mutex_clock);
		return;
	}
	log_debug(
			logger_planificacion,
			"[R%i-P%i] Bloqueando por descanso...",
			pedido->repartidor->id,
			pedido->pedido_id
	);
	planif_encolar_BLOCK(pedido);
	// log_debug(
	//     logger_planificacion,
	//     "[REP_%2i] Seteando tiempo de descanso restante a %i",
	//     pedido->repartidor->id,
	//     pedido->repartidor->tiempoDescanso
	// );
	for (
			pedido->repartidor->tiempoDescanso_restante = pedido->repartidor->tiempoDescanso;
			pedido->repartidor->tiempoDescanso_restante > 0;
			pedido->repartidor->tiempoDescanso_restante--
	) {
		// sleep(cfval_retardoCicloCPU);
		pthread_mutex_trylock(pedido->mutex_clock);
		pthread_mutex_lock(pedido->mutex_clock);
		log_debug(
				logger_planificacion,
				"[R%i-P%i] Ciclo descanso consumido (restan %i)",
				pedido->repartidor->id,
				pedido->pedido_id,
				pedido->repartidor->tiempoDescanso_restante - 1
		);
	}
	log_debug(
			logger_planificacion,
			"[R%i-P%i] Descanso terminado",
			pedido->repartidor->id,
			pedido->pedido_id
	);
	pedido->repartidor->frecuenciaDescanso_restante = pedido->repartidor->frecuenciaDescanso;
	planif_encolar_READY(pedido);
	log_debug(
			logger_planificacion,
			"[R%i-P%i] Esperando unlock EXEC",
			pedido->repartidor->id,
			pedido->pedido_id
	);
	// pthread_mutex_trylock(pedido->mutex_EXEC);
	// log_debug(
	//     logger_planificacion,
	//     "[R%i-P%i] Post trylock mutex EXEC",
	//     pedido->repartidor->id,
	//     pedido->pedido_id
	// );
	pthread_mutex_lock(pedido->mutex_EXEC);
	log_debug(
			logger_planificacion,
			"[R%i-P%i] Post lock mutex EXEC",
			pedido->repartidor->id,
			pedido->pedido_id
	);
	pthread_mutex_trylock(pedido->mutex_clock);
	log_debug(
			logger_planificacion,
			"[R%i-P%i] Post trylock mutex clock",
			pedido->repartidor->id,
			pedido->pedido_id
	);
	pthread_mutex_lock(pedido->mutex_clock);
	log_debug(
			logger_planificacion,
			"[R%i-P%i] Post lock mutex clock",
			pedido->repartidor->id,
			pedido->pedido_id
	);
}

void guardar_nuevoRest(m_restaurante* mensaje_rest, int socket) { // TODO: commons
	t_restaurante* restaurante = malloc(sizeof(t_restaurante));
	restaurante->nombre = string_duplicate(mensaje_rest->nombre.nombre); // TODO: string duplicate / check free
	restaurante->pos_x = mensaje_rest->posicion.x;
	restaurante->pos_y = mensaje_rest->posicion.y;

	log_debug(logger_mensajes, "Socket pre guardado de restaurante %s: %i",
			restaurante->nombre,
			socket
	);
	restaurante->socket = socket;
	log_debug(logger_mensajes, "Socket post guardado de restaurante %s: %i",
			restaurante->nombre,
			restaurante->socket
	);

	restaurante->mutex = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(restaurante->mutex, NULL);

	restaurante->q_mtx = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(restaurante->q_mtx, NULL);

	restaurante->q_sem = malloc(sizeof(sem_t));
	sem_init(restaurante->q_sem, 0, 0);

	restaurante->q = list_create();

	restaurante->q_admin = malloc(sizeof(pthread_t));
	pthread_create(restaurante->q_admin, NULL, qr_admin, restaurante);

	pthread_mutex_lock(&mutex_lista_restaurantes);
	list_add(restaurantes, restaurante);
	// TODO: logging
	pthread_mutex_unlock(&mutex_lista_restaurantes);

	free_struct_mensaje(mensaje_rest, POSICION_RESTAUNTE);
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
		log_debug(
				logger_mensajes,
				"[MENSJS]: RESTO DEFAULT"
		);
		pthread_mutex_unlock(&mutex_lista_restaurantes);
		restaurante = malloc(sizeof(t_nombre));
		restaurante->nombre = string_duplicate(resto_default->nombre);
		list_add(nombresRestConectados, restaurante);
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
	unsigned ciclo_display_counter = 0;
	while (true) {
		pthread_mutex_lock(&mutex_cola_BLOCK);
		pthread_mutex_lock(&mutex_pedidosEXEC);
		pthread_mutex_lock(&mutex_cola_READY);
		log_debug(
				logger_planificacion,
				"[CLOCK]: -------------------------------------------------------- Ciclo %i",
				++ciclo_display_counter
		);
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
		for (
				unsigned index_READY = 0;
				index_READY < list_size(cola_READY);
				index_READY++
		) {
			((t_pedido*)list_get(cola_READY, index_READY))->hrrn_tiempoEsperaREADY++;
		}
		pthread_mutex_unlock(&mutex_cola_BLOCK);
		pthread_mutex_unlock(&mutex_pedidosEXEC);
		pthread_mutex_unlock(&mutex_cola_READY);
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

void* fhilo_servidor(void* arg) {
	int conexion_servidor;
	conexion_servidor = iniciar_servidor(cfval_puertoEscucha);
	log_debug(logger_mensajes, "[MENSJS]: fhilo_servidor, pre esperar_cliente");
	while(1) {
		esperar_cliente(conexion_servidor);
	}
}

void esperar_cliente(int servidor){
	struct sockaddr_in direccion_cliente;

	unsigned int tam_direccion = sizeof(struct sockaddr_in);

	log_debug(logger_mensajes, "[MENSJS]: arranca esperar cliente");
	int cliente = accept (servidor, (void*) &direccion_cliente, &tam_direccion);

	log_debug(logger_mensajes, "[MENSJS]: conexion aceptada");

	pthread_t hilo;

	pthread_mutex_lock(&mutex_hilos);
	log_debug(logger_mensajes, "[MENSJS]: mtx hilos lockeado");
	list_add(hilos, &hilo);
	pthread_mutex_unlock(&mutex_hilos);

	log_debug(logger_mensajes, "[MENSJS]: creando hilo serve_client");
	pthread_create(&hilo,NULL,(void*)serve_client,cliente);
	pthread_detach(hilo);

}

void serve_client(int socket){
	int rec;
	int cod_op = -1;
	// while(1){
	log_debug(logger_mensajes, "[MENSJS]: pre recibir op_code");
	rec = recv(socket, &cod_op, sizeof(op_code), MSG_WAITALL);
	if(rec == -1 || rec == 0 ){
		cod_op = -1;
		log_debug(logger_mensajes, "[MENSJS]: Fin de conexion: recv = %i", rec);
		//			pthread_mutex_lock(&logger_mutex);
		//			log_info(logger,"Se desconecto el proceso con id: %d",socket);
		//			pthread_mutex_unlock(&logger_mutex);
		pthread_exit(NULL);
	}
	log_debug(
			logger_mensajes,
			"[MENSJS]: post recibir op_code: %i-%s",
			cod_op,
			op_code_to_string(cod_op)
	);
	log_debug(logger_mensajes, "[MENSJS]: pre process_request");
	process_request(cod_op, socket);
	// }
}

void process_request(int cod_op, int cliente_fd) {
	int size = 0;
	int error = FLAG_OK;
	void* mensaje = NULL;
	int rec;
	uint32_t cliente_id;

	// TODO: provisorio?
	log_debug(logger_mensajes, "[MENSJS]: en process_request, por hacer recv de id");
	rec = recv(cliente_fd, &cliente_id, sizeof(uint32_t), MSG_WAITALL);
	log_debug(logger_mensajes, "[MENSJS]: en process_request, post hacer recv de id: %i", cliente_id);

	if(rec == -1 || rec == 0 ){
		cod_op = -1;
		//			pthread_mutex_lock(&logger_mutex);
		//			log_info(logger,"Se desconecto el proceso con id: %d",socket);
		//			pthread_mutex_unlock(&logger_mutex);
		log_debug(logger_mensajes, "[MENSJS]: Al hacer recv, hubo un error o la conexion ya se cerro", rec);
		pthread_exit(NULL);
	}

	// log_debug(logger_mensajes, "[MENSJS]: pre comparacion sospechosa");
	// log_debug(logger_mensajes, "[MENSJS]: struct code: %i", op_code_to_struct_code(cod_op));

	// if(op_code_to_struct_code(cod_op) != STRC_MENSAJE_VACIO){

	// log_debug(logger_mensajes, "[MENSJS]: entro en if porque el mje no es vacio");
	void* buffer = recibir_mensaje(cliente_fd, &error);
	log_debug(logger_mensajes, "[MENSJS]: mje recibido");
	if (error) {
		log_debug(logger_mensajes, "[MENSJS]: Hubo un error al recibir el mje.");
		pthread_exit(NULL);
	}
	log_debug(logger_mensajes, "[MENSJS] puntero del buffer: %x", buffer);
	mensaje = deserializar_mensaje(buffer, cod_op);
	loggear_mensaje_recibido(mensaje, cod_op, logger_mensajes);
	log_debug(logger_mensajes, "[MENSJS]: mje deserializado");

	// }

	// TODO loggear_mensaje_recibido(mensaje_deserializado, cod_op, logger_mensajes);

	switch (cod_op) {

	// de CLIENTE:
	case POSICION_CLIENTE:
		gestionar_POSICION_CLIENTE(cliente_id, mensaje, cliente_fd);
		break;
	case CONSULTAR_RESTAURANTES:
		gestionar_CONSULTAR_RESTAURANTES(cliente_fd);
		break;
	case SELECCIONAR_RESTAURANTE:
		gestionar_SELECCIONAR_RESTAURANTE(mensaje, cliente_fd);
		break;
	case CONSULTAR_PLATOS:
		gestionar_CONSULTAR_PLATOS(cliente_id, cliente_fd);
		break;
	case CREAR_PEDIDO:
		gestionar_CREAR_PEDIDO(cliente_id, cliente_fd);
		break;
	case AGREGAR_PLATO:
		gestionar_AGREGAR_PLATO(mensaje, cliente_id, cliente_fd);
		break;
	case CONFIRMAR_PEDIDO:
		gestionar_CONFIRMAR_PEDIDO(mensaje, cliente_fd, cliente_id);
		break;
	case CONSULTAR_PEDIDO:
		gestionar_CONSULTAR_PEDIDO(mensaje, cliente_fd, cliente_id);
		break;

		// de RESTAURANT:
	case POSICION_RESTAUNTE:
		guardar_nuevoRest(mensaje, cliente_fd);
		break;

	case PLATO_LISTO:
		gestionar_PLATO_LISTO(mensaje, cliente_fd);
		break;

		// TODO: liberar mensaje aca o dentro de cada f?
	}
}

void gestionar_POSICION_CLIENTE(int cliente_id, t_coordenadas* posicion, int socket_cliente) {
	t_cliente* cliente;

	log_debug(
			logger_mensajes,
			"[MENSJS]: Llego pos. (%i, %i) del cliente %i (INTENTO HANDSHAKE)",
			posicion->x,
			posicion->y,
			cliente_id
	);

	pthread_mutex_lock(&mutex_lista_clientes);

//	if (exists_cliente(cliente_id)) {
//		log_debug(
//				logger_mensajes,
//				"[MENSJS]: Ya existe un cliente por el mismo id; se rechaza el registro (ERROR)"
//		);
//		responder_ERROR(socket_cliente);
//		return;
//	}

	cliente = malloc(sizeof(t_cliente));
	cliente->id = cliente_id;
	cliente->pos_x = posicion->x;
	cliente->pos_y = posicion->y;
	cliente->restaurante_seleccionado = NULL;
	cliente->socket = socket_cliente;
	cliente->mutex = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(cliente->mutex, NULL);
	pthread_mutex_lock(cliente->mutex);

	list_add(clientes, cliente);
	// TODO: logging
	pthread_mutex_unlock(&mutex_lista_clientes);

	log_debug(
			logger_mensajes,
			"[MENSJS]: Contestando handshake OK..."
	);
	responder_confirm(socket_cliente, true, RTA_POSICION_CLIENTE);
	log_debug(
			logger_mensajes,
			"[MENSJS]: Handshake contestado"
	);

	pthread_mutex_unlock(cliente->mutex);
}

void gestionar_CONSULTAR_RESTAURANTES(int socket_cliente) {
	t_mensaje* mensaje = malloc(sizeof(t_mensaje));
	t_restaurante_y_plato* restaurantes = malloc(sizeof(t_restaurante_y_plato));

	log_debug(
			logger_mensajes,
			"[MENSJS]: Gestionando CONSULTAR_RESTAURANTES:"
	);

	restaurantes->nombres = get_nombresRestConectados();
	for (
			unsigned index_rests = 0;
			index_rests < list_size(restaurantes->nombres);
			index_rests++
	) {
		log_debug(
				logger_mensajes,
				"[MENSJS]: \t%i: %s",
				index_rests,
				((t_nombre*)list_get(restaurantes->nombres, index_rests))->nombre
		);
	}
	log_debug(
			logger_mensajes,
			"[MENSJS]: Cant. de elems.: %i",
			list_size(restaurantes->nombres)
	);

	mensaje->tipo_mensaje = RTA_CONSULTAR_RESTAURANTES;
	mensaje->id = cfval_id;
	mensaje->parametros = restaurantes;
	log_debug(
			logger_mensajes,
			"[MENSJS]: Enviando RTA_CONSULTAR_RESTAURANTES..."
	);
	enviar_mensaje(mensaje, socket_cliente);
	log_debug(
			logger_mensajes,
			"[MENSJS]: RTA_CONSULTAR_RESTAURANTES enviado"
	);
	loggear_mensaje_enviado(mensaje->parametros, mensaje->tipo_mensaje, logger_mensajes);
	free_struct_mensaje(mensaje->parametros, mensaje->tipo_mensaje);
	free(mensaje);
}

void responder_confirm(int socket_cliente, bool ok_fail, int cod_op) {
	t_mensaje* mensaje = malloc(sizeof(t_mensaje));
	mensaje->tipo_mensaje = cod_op;
	mensaje->id = cfval_id;
	uint32_t* confirmacion = malloc(sizeof(uint32_t));
	*confirmacion = ok_fail ? 1 : 0;
	mensaje->parametros = confirmacion;
	log_debug(logger_mensajes, "[MENSJS]: pre enviar confirm");
	enviar_mensaje(mensaje, socket_cliente);
	log_debug(logger_mensajes, "[MENSJS]: post enviar confirm: %s - %s",
			op_code_to_string(cod_op),
			ok_fail ? "OK" : "FAIL"
	);
	free_struct_mensaje(mensaje->parametros, mensaje->tipo_mensaje);
	free(mensaje);
}

void gestionar_SELECCIONAR_RESTAURANTE(m_seleccionarRestaurante* seleccion, int socket_cliente) {
	t_cliente* cliente_seleccionante;

	cliente_seleccionante = get_cliente_porSuID(seleccion->cliente);
	if (!cliente_seleccionante) {
		log_debug(logger_mensajes, "[MENSJS]: No se encontro el puntero del cliente %i -> FAIL", seleccion->cliente);
		responder_confirm(socket_cliente, false, RTA_SELECCIONAR_RESTAURANTE);
		return;
	}
	pthread_mutex_lock(cliente_seleccionante->mutex);
	log_debug(logger_mensajes, "[MENSJS]: Se encontro el puntero del cliente %i",
			cliente_seleccionante->id
	);
	cliente_seleccionante->restaurante_seleccionado = get_restaurante(seleccion->restaurante.nombre);

	if (!(cliente_seleccionante->restaurante_seleccionado)) {
		log_debug(
				logger_mensajes,
				"[MENSJS]: No se encontro el restaurante %s; se responde FAIL",
				seleccion->restaurante.nombre
		);
		responder_confirm(socket_cliente, false, RTA_SELECCIONAR_RESTAURANTE);
		pthread_mutex_unlock(cliente_seleccionante->mutex);
		return;
	}

	log_debug(logger_mensajes, "[MENSJS]: se encontro rest. %s; ahora seleccionado -> OK", cliente_seleccionante->restaurante_seleccionado->nombre);

	responder_confirm(socket_cliente, true, RTA_SELECCIONAR_RESTAURANTE);

	pthread_mutex_unlock(cliente_seleccionante->mutex);
}

void gestionar_CONSULTAR_PLATOS(int cliente_id, int socket_cliente) {
	t_mensaje* mensaje;
	t_nombre* nombre_rest_consultado;
	t_restaurante* restaurante_consultado;
	t_cliente* cliente_consultor;
	qr_form_t* form;

	log_debug(
			logger_mensajes,
			"[MENSJS]: Gestionando CONSULTAR_PLATOS:"
	);

	cliente_consultor = get_cliente_porSuID(cliente_id);
	if (!cliente_consultor) {
		log_debug(logger_mensajes, "[MENSJS]: No se encontro el puntero del cliente %i -> ERROR", cliente_id);
		responder_ERROR(socket_cliente);
		return;
	}
	log_debug(logger_mensajes, "[MENSJS]: Se encontro el puntero del cliente %i", cliente_consultor->id);
	pthread_mutex_lock(cliente_consultor->mutex);

	restaurante_consultado = cliente_consultor->restaurante_seleccionado;
	if (!restaurante_consultado) {
		log_debug(
				logger_mensajes,
				"[MENSJS]: El cliente no tiene restaurante seleccionado, se contesta ERROR"
		);
		responder_ERROR(socket_cliente);
		pthread_mutex_unlock(cliente_consultor->mutex);
		return;
	} else if (restaurante_consultado == resto_default) {
		log_debug(
				logger_mensajes,
				"[MENSJS]: El cliente consulto platos por Resto Default, se responde directamente"
		);
		mensaje = malloc(sizeof(t_mensaje));
		mensaje->tipo_mensaje = RTA_CONSULTAR_PLATOS;
		mensaje->id = cfval_id;
		mensaje->parametros = platos_default_enviable;
		log_debug(
				logger_mensajes,
				"[MENSJS]: Pre enviar mensaje"
		);
		enviar_mensaje(mensaje, socket_cliente);
		log_debug(
				logger_mensajes,
				"[MENSJS]: Post enviar mensaje"
		);
		free(mensaje);
		pthread_mutex_unlock(cliente_consultor->mutex);
		return;
	}

	nombre_rest_consultado = malloc(sizeof(t_nombre));
	nombre_rest_consultado->nombre="";
	mensaje = malloc(sizeof(t_mensaje));
	mensaje->tipo_mensaje = CONSULTAR_PLATOS;
	mensaje->parametros = nombre_rest_consultado;

	form = qr_request(mensaje, restaurante_consultado);

	if (*(form->error_flag)) {
		log_debug(logger_mensajes, "[MENSJS]: Form con error flag -> se responde ERROR");
		responder_ERROR(socket_cliente);
		pthread_mutex_unlock(cliente_consultor->mutex);
		qr_free_form(form);
		return; // TODO: liberar la memoria previa a este if dentro de aca
	}

	switch (form->m_recibir->tipo_mensaje) {
	case ERROR:
		log_debug(logger_mensajes, "[MENSJS]: Rest. responde ERROR -> se responde ERROR");
		responder_ERROR(socket_cliente);
		break;
	case RTA_CONSULTAR_PLATOS:
		log_debug(logger_mensajes, "[MENSJS]: Rest. responde como se esperaba");
		enviar_mensaje(form->m_recibir, socket_cliente);
		loggear_mensaje_enviado(form->m_recibir->parametros, form->m_recibir->tipo_mensaje, logger_mensajes);
		break;
	default:
		log_debug(logger_mensajes, "[MENSJS]: Rest. no responde como se esperaba -> se responde ERROR");
		responder_ERROR(socket_cliente);
	}

	pthread_mutex_unlock(cliente_consultor->mutex);
	qr_free_form(form);
	// TODO: liberar la memoria
}

unsigned resto_default_get_id(void) {
	unsigned id;
	pthread_mutex_lock(&resto_default_id_serial_mtx);
	id = resto_default_id_serial++;
	pthread_mutex_unlock(&resto_default_id_serial_mtx);
	return id;
}

void gestionar_CREAR_PEDIDO(int cliente_id, int socket_cliente) {
	t_cliente* cliente_en_cuestion;
	t_restaurante* resto_en_cuestion;
	t_mensaje* mensaje;
	qr_form_t* form;
	int socket_comanda;
	t_nombre_y_id* params;
	t_mensaje* rta_comanda;
	int pedido_id;

	// buscar rest de ese cliente
	cliente_en_cuestion = get_cliente_porSuID(cliente_id); // TODO: error check
	if (!cliente_en_cuestion) {
		log_debug(logger_mensajes, "[MENSJS]: Cliente %i no registrado -> se responde ERROR", cliente_id);
		responder_ERROR(socket_cliente);
	}
	pthread_mutex_lock(cliente_en_cuestion->mutex);

	resto_en_cuestion = cliente_en_cuestion->restaurante_seleccionado; // TODO: ojo el mutex del socket del rest.
	if (!resto_en_cuestion) {
		log_debug(logger_mensajes, "[MENSJS]: Cliente %i no tiene rest. selecc. -> ERROR", cliente_id);
		responder_ERROR(socket_cliente);
		pthread_mutex_unlock(cliente_en_cuestion->mutex);
		return;
	}
	if (resto_en_cuestion == resto_default) {
		pedido_id = resto_default_get_id();
		log_debug(logger_mensajes, "[MENSJS]: Cliente %i tenia RestoDefault (nuevo ped. %i)",
				cliente_id,
				pedido_id
		);
	} else {
		// mandarle el crear pedido
		log_debug(logger_mensajes, "[MENSJS]: Cliente %i tenia %s seleccionado",
				cliente_id,
				resto_en_cuestion->nombre
		);
		mensaje = malloc(sizeof(t_mensaje));
		mensaje->tipo_mensaje = CREAR_PEDIDO;
		mensaje->id = cfval_id;
		mensaje->parametros = NULL;
		form = qr_request(mensaje, resto_en_cuestion);
		if (*(form->error_flag)) {
			log_debug(logger_mensajes, "[MENSJS]: Form con error flag -> se responde ERROR");
			responder_ERROR(socket_cliente);
			qr_free_form(form);
			pthread_mutex_unlock(cliente_en_cuestion->mutex);
			return;
		}
		switch (form->m_recibir->tipo_mensaje) {
		case ERROR:
			log_debug(logger_mensajes, "[MENSJS]: Rest. respondio ERROR -> se responde ERROR a cli");
			responder_ERROR(socket_cliente);
			qr_free_form(form);
			pthread_mutex_unlock(cliente_en_cuestion->mutex);
			return;
		case RTA_CREAR_PEDIDO:
			log_debug(logger_mensajes, "[MENSJS]: Rest. respondio como se esperaba");
			pedido_id = *((uint32_t*)(form->m_recibir->parametros));
			qr_free_form(form);
			break;
			// mensaje->tipo_mensaje = RTA_CREAR_PEDIDO;
			// mensaje->id = cfval_id;
			// mensaje->parametros = (uint32_t*)(form->m_recibir->parametros);
			// enviar_mensaje(mensaje, socket_cliente);
			// free(mensaje);
		default:
			log_debug(logger_mensajes, "[MENSJS]: Rest. no respondio como se esperaba -> ERROR a cli");
			loggear_mensaje_recibido(form->m_recibir->parametros, form->m_recibir->tipo_mensaje, logger_mensajes);
			responder_ERROR(socket_cliente);
			qr_free_form(form);
			pthread_mutex_unlock(cliente_en_cuestion->mutex);
			return;
		}
	}

	params = malloc(sizeof(t_nombre_y_id));
	params->id = pedido_id;
	params->nombre.nombre = resto_en_cuestion->nombre;
	rta_comanda = mensajear_comanda(GUARDAR_PEDIDO, params, true);

	if (!rta_comanda) {
		log_debug(logger_mensajes, "[MENSJS]: Hubo un error al recibir rta. de comanda -> ERROR a cli");
		responder_ERROR(socket_cliente);
		pthread_mutex_unlock(cliente_en_cuestion->mutex);
		return;
	}

	switch (rta_comanda->tipo_mensaje) {
	case ERROR:
		log_debug(logger_mensajes, "[MENSJS]: Comanda respondio ERROR -> se responde ERROR a cli");
		responder_ERROR(socket_cliente);
		break;
	case RTA_GUARDAR_PEDIDO:
		if (*(int*)(rta_comanda->parametros)) { // -------------------------------- OK
			log_debug(logger_mensajes, "[MENSJS]: Comanda respondio OK");
			mensaje = malloc(sizeof(t_mensaje));
			mensaje->tipo_mensaje = RTA_CREAR_PEDIDO;
			mensaje->id = cfval_id;
			mensaje->parametros = &pedido_id;
			log_debug(
					logger_mensajes,
					"[MENSJS] Contestando OK al cliente..."
			);
			enviar_mensaje(mensaje, socket_cliente);
			log_debug(
					logger_mensajes,
					"[MENSJS] OK al cliente contestado!"
			);
			free(mensaje);
		} else { // --------------------------------------------------------- FAIL
			log_debug(logger_mensajes, "[MENSJS]: Comanda respondio ERROR -> ERROR a cli");
			responder_ERROR(socket_cliente);
		}
		break;
	default:
		log_debug(logger_mensajes, "[MENSJS]: Comanda no respondio como se esperaba -> ERROR a cli");
		responder_ERROR(socket_cliente);
		break;
	}
	free_struct_mensaje(rta_comanda->parametros, rta_comanda->tipo_mensaje);
	free(rta_comanda);
	pthread_mutex_unlock(cliente_en_cuestion->mutex);
}

void gestionar_AGREGAR_PLATO(t_nombre_y_id* plato, int cliente_id, int socket_cliente) {
	t_cliente* cliente_en_cuestion;
	t_restaurante* resto_en_cuestion;
	t_mensaje* mensaje;
	t_mensaje* rta_comanda;
	qr_form_t* form = NULL;
	m_guardarPlato* plato_comanda = NULL;

	// buscar rest de ese cliente
	cliente_en_cuestion = get_cliente_porSuID(cliente_id);
	if (!cliente_en_cuestion) {
		log_debug(logger_mensajes, "[MENSJS]: El cliente %i no esta registrado -> FAIL a cli", cliente_id);
		responder_confirm(socket_cliente, false, RTA_AGREGAR_PLATO);
		return;
	}
	pthread_mutex_lock(cliente_en_cuestion->mutex);

	resto_en_cuestion = cliente_en_cuestion->restaurante_seleccionado;
	if (!resto_en_cuestion) {
		log_debug(logger_mensajes, "[MENSJS]: El cliente %i no selecciono rest. -> FAIL a cli", cliente_id);
		responder_confirm(socket_cliente, false, RTA_AGREGAR_PLATO);
		pthread_mutex_unlock(cliente_en_cuestion->mutex);
		return;
	}
	if (resto_en_cuestion != resto_default) {
		mensaje = malloc(sizeof(t_mensaje));
		mensaje->tipo_mensaje = AGREGAR_PLATO;
		mensaje->id = cfval_id;
		mensaje->parametros = plato;
		form = qr_request(mensaje, resto_en_cuestion);
		if (*(form->error_flag)) {
			log_debug(logger_mensajes, "[MENSJS]: Form con error flag -> FAIL a cli", cliente_id);
			responder_confirm(socket_cliente, false, RTA_AGREGAR_PLATO);
			pthread_mutex_unlock(cliente_en_cuestion->mutex);
			qr_free_form(form);
			return;
		}
		switch (form->m_recibir->tipo_mensaje) {
		case ERROR:
			log_debug(logger_mensajes, "[MENSJS]: Rest. responde ERROR -> FAIL a cli", cliente_id);
			responder_confirm(socket_cliente, false, RTA_AGREGAR_PLATO);
			pthread_mutex_unlock(cliente_en_cuestion->mutex);
			qr_free_form(form);
			return;
		case RTA_AGREGAR_PLATO:
			if (!*(int*)(form->m_recibir->parametros)) { // ------------------- FAIL
				log_debug(logger_mensajes, "[MENSJS]: Rest. responde FAIL -> FAIL a cli", cliente_id);
				responder_confirm(socket_cliente, false, RTA_AGREGAR_PLATO);
				pthread_mutex_unlock(cliente_en_cuestion->mutex);
				qr_free_form(form);
				return;
			} // -------------------------------------------------------- OK
			log_debug(logger_mensajes, "[MENSJS]: Rest. responde OK", cliente_id);
			break;
		default:
			log_debug(logger_mensajes, "[MENSJS]: Rest. responde ERROR -> FAIL a cli", cliente_id);
			responder_confirm(socket_cliente, false, RTA_AGREGAR_PLATO);
			pthread_mutex_unlock(cliente_en_cuestion->mutex);
			qr_free_form(form);
			return;
		}
	}

	// mandarle guardar plato a comanda
	plato_comanda = malloc(sizeof(m_guardarPlato));
	plato_comanda->restaurante.nombre = string_duplicate(resto_en_cuestion->nombre);
	plato_comanda->idPedido = plato->id;
	plato_comanda->comida.nombre = string_duplicate(plato->nombre.nombre);
	plato_comanda->cantidad = 1;
	rta_comanda = mensajear_comanda(GUARDAR_PLATO, plato_comanda, true);

	log_debug(logger_mensajes, "[MENSJS] rta_comanda: %x", rta_comanda);
	log_debug(logger_mensajes, "[MENSJS] rta_comanda->parametros: %x", rta_comanda->parametros);

	qr_free_form(form);

	// proc respuesta
	if (!rta_comanda) {
		log_debug(logger_mensajes, "[MENSJS]: Error recibiendo rta. comanda -> FAIL a cli", cliente_id);
		responder_confirm(socket_cliente, false, RTA_AGREGAR_PLATO);
	} else {
		switch (rta_comanda->tipo_mensaje) {
		case ERROR:
			log_debug(logger_mensajes, "[MENSJS]: Comanda respondio ERROR -> FAIL a cli", cliente_id);
			responder_confirm(socket_cliente, false, RTA_AGREGAR_PLATO);
			break;
		case RTA_GUARDAR_PLATO:
			if (*(int*)(rta_comanda->parametros)) {
				log_debug(logger_mensajes, "[MENSJS]: Comanda responde OK -> OK a cli", cliente_id);
				responder_confirm(socket_cliente, true, RTA_AGREGAR_PLATO);
			} else {
				log_debug(logger_mensajes, "[MENSJS]: Comanda respondio FAIL -> FAIL a cli", cliente_id);
				responder_confirm(socket_cliente, false, RTA_AGREGAR_PLATO);
			}
			break;
		default:
			log_debug(logger_mensajes, "[MENSJS]: Respuesta inesperada de comanda -> FAIL a cli", cliente_id);
			responder_confirm(socket_cliente, false, RTA_AGREGAR_PLATO);
		}
	}

	pthread_mutex_unlock(cliente_en_cuestion->mutex);
	free_struct_mensaje(rta_comanda->parametros, rta_comanda->tipo_mensaje);
	free(rta_comanda);
}

void gestionar_CONFIRMAR_PEDIDO(t_nombre_y_id* pedido, int socket_cliente, int cliente_id) {
	t_mensaje* rta_obtener_pedido;
	t_restaurante* rest_a_conf;
	t_mensaje* mje_cprest;
	t_mensaje* rta_cp_comanda;
	t_cliente* cliente_confirmante;
	qr_form_t* form;
	t_nombre_y_id* nombre_id_revisado;
	t_nombre_y_id* pedido_comanda;

	cliente_confirmante = get_cliente_porSuID(cliente_id);
	if (!cliente_confirmante) {
		log_debug(logger_mensajes, "[MENSJS] El cliente %i no esta registrado -> FAIL", cliente_id);
		responder_confirm(socket_cliente, false, RTA_CONFIRMAR_PEDIDO);
		return;
	}
	pthread_mutex_lock(cliente_confirmante->mutex);

	rest_a_conf = cliente_confirmante->restaurante_seleccionado;
	if (!rest_a_conf) {
		log_debug(logger_mensajes, "[MENSJS] El cliente no selecciono rest. -> FAIL", cliente_id);
		responder_confirm(socket_cliente, false, RTA_CONFIRMAR_PEDIDO);
		pthread_mutex_unlock(cliente_confirmante->mutex);
		return;
	}

	nombre_id_revisado = malloc(sizeof(t_nombre_y_id));
	nombre_id_revisado->id = pedido->id;
	nombre_id_revisado->nombre.nombre = string_duplicate(rest_a_conf->nombre);
	rta_obtener_pedido = mensajear_comanda(OBTENER_PEDIDO, nombre_id_revisado, false);

	if (!rta_obtener_pedido) {
		log_debug(logger_mensajes, "[MENSJS] Hubo un error al recibir respuesta de comanda -> FAIL a cli");
		responder_confirm(socket_cliente, false, RTA_CONFIRMAR_PEDIDO);
		free_struct_mensaje(rta_obtener_pedido->parametros, rta_obtener_pedido->tipo_mensaje);
		free(rta_obtener_pedido);
		free_struct_mensaje(pedido, CONFIRMAR_PEDIDO);
		pthread_mutex_unlock(cliente_confirmante->mutex);
		return;
	}
	switch (rta_obtener_pedido->tipo_mensaje) {
	case ERROR:
		log_debug(logger_mensajes, "[MENSJS] Comanda respondio ERROR -> FAIL a cli");
		responder_confirm(socket_cliente, false, RTA_CONFIRMAR_PEDIDO);
		free_struct_mensaje(rta_obtener_pedido->parametros, rta_obtener_pedido->tipo_mensaje);
		free(rta_obtener_pedido);
		free_struct_mensaje(pedido, CONFIRMAR_PEDIDO);
		pthread_mutex_unlock(cliente_confirmante->mutex);
		return;

	case RTA_OBTENER_PEDIDO:
		log_debug(logger_mensajes, "[MENSJS] Comanda respondio RTA_OBTENER_PEDIDO");

		free_struct_mensaje(rta_obtener_pedido->parametros, rta_obtener_pedido->tipo_mensaje);
		free(rta_obtener_pedido);

		if (rest_a_conf == resto_default) {
			log_debug(logger_mensajes, "[MENSJS] El rest. seleccionado es RestoDefault");
			rta_cp_comanda = mensajear_comanda(CONFIRMAR_PEDIDO, nombre_id_revisado, false);
			if (!rta_cp_comanda) {
				log_debug(logger_mensajes, "[MENSJS] Hubo error al recibir rta. comanda -> FAIL a cli");
				responder_confirm(socket_cliente, false, RTA_CONFIRMAR_PEDIDO);
				free_struct_mensaje(pedido, CONFIRMAR_PEDIDO);
				free_struct_mensaje(rta_cp_comanda->parametros, rta_cp_comanda->tipo_mensaje);
				free(rta_cp_comanda);
				pthread_mutex_unlock(cliente_confirmante->mutex);
				return;
			}
			switch (rta_cp_comanda->tipo_mensaje) {
			case ERROR:
				log_debug(logger_mensajes, "[MENSJS] Comanda respondio ERROR -> FAIL a cli");
				responder_confirm(socket_cliente, false, RTA_CONFIRMAR_PEDIDO);
				free_struct_mensaje(pedido, CONFIRMAR_PEDIDO);
				free_struct_mensaje(rta_cp_comanda->parametros, rta_cp_comanda->tipo_mensaje);
				free(rta_cp_comanda);
				pthread_mutex_unlock(cliente_confirmante->mutex);
				return;
			case RTA_CONFIRMAR_PEDIDO:
				if (!*(int*)(rta_cp_comanda->parametros)) {
					log_debug(logger_mensajes, "[MENSJS] Comanda respondio FAIL -> FAIL a cli");
					responder_confirm(socket_cliente, false, RTA_CONFIRMAR_PEDIDO);
					free_struct_mensaje(pedido, CONFIRMAR_PEDIDO);
					free_struct_mensaje(rta_cp_comanda->parametros, rta_cp_comanda->tipo_mensaje);
					free(rta_cp_comanda);
					pthread_mutex_unlock(cliente_confirmante->mutex);
					return;
				}
				log_debug(logger_mensajes, "[MENSJS] Comanda respondio OK");
				planif_nuevoPedido(cliente_id, pedido->id);
				responder_confirm(socket_cliente, true, RTA_CONFIRMAR_PEDIDO);
				free_struct_mensaje(pedido, CONFIRMAR_PEDIDO);
				free_struct_mensaje(rta_cp_comanda->parametros, rta_cp_comanda->tipo_mensaje);
				free(rta_cp_comanda);
				pthread_mutex_unlock(cliente_confirmante->mutex);
				return;
			default:
				log_debug(logger_mensajes, "[MENSJS] Comanda no respondio lo esperado -> FAIL a cli");
				responder_confirm(socket_cliente, false, RTA_CONFIRMAR_PEDIDO);
				free_struct_mensaje(pedido, CONFIRMAR_PEDIDO);
				free_struct_mensaje(rta_cp_comanda->parametros, rta_cp_comanda->tipo_mensaje);
				free(rta_cp_comanda);
				pthread_mutex_unlock(cliente_confirmante->mutex);
				return;
			}
		}



		mje_cprest = malloc(sizeof(t_mensaje));
		mje_cprest->tipo_mensaje = CONFIRMAR_PEDIDO;
		mje_cprest->id = cfval_id;
		pedido_comanda = malloc(sizeof(t_nombre_y_id));
		pedido_comanda->id = pedido->id;
		pedido_comanda->nombre.nombre = string_duplicate(rest_a_conf->nombre);
		mje_cprest->parametros = pedido_comanda;
		free_struct_mensaje(pedido, CONFIRMAR_PEDIDO);
		form = qr_request(mje_cprest, rest_a_conf);
		if (*(form->error_flag)) {
			log_debug(logger_mensajes, "[MENSJS] Form con error flag -> FAIL a cli");
			responder_confirm(socket_cliente, false, RTA_CONFIRMAR_PEDIDO);
			qr_free_form(form);
			free_struct_mensaje(pedido_comanda, CONFIRMAR_PEDIDO);
			pthread_mutex_unlock(cliente_confirmante->mutex);
			return;
		}
		switch (form->m_recibir->tipo_mensaje) {
		case ERROR:
			log_debug(logger_mensajes, "[MENSJS] Rest. respondio ERROR -> FAIL a cli");
			responder_confirm(socket_cliente, false, RTA_CONFIRMAR_PEDIDO);
			qr_free_form(form);
			free_struct_mensaje(pedido_comanda, CONFIRMAR_PEDIDO);
			pthread_mutex_unlock(cliente_confirmante->mutex);
			return;
		case RTA_CONFIRMAR_PEDIDO:
			if (!*(int*)(form->m_recibir->parametros)) {
				log_debug(logger_mensajes, "[MENSJS] Rest. respondio FAIL -> FAIL a cli");
				responder_confirm(socket_cliente, false, RTA_CONFIRMAR_PEDIDO);
				qr_free_form(form);
				free_struct_mensaje(pedido_comanda, CONFIRMAR_PEDIDO);
				pthread_mutex_unlock(cliente_confirmante->mutex);
				return;
			}
			log_debug(logger_mensajes, "[MENSJS] Rest. respondio OK");
			qr_free_form(form);
			rta_cp_comanda = mensajear_comanda(CONFIRMAR_PEDIDO, pedido_comanda, false);
			if (!rta_cp_comanda) {
				log_debug(logger_mensajes, "[MENSJS] Hubo error al recibir rta. comanda -> FAIL a cli");
				responder_confirm(socket_cliente, false, RTA_CONFIRMAR_PEDIDO);
				free_struct_mensaje(pedido_comanda, CONFIRMAR_PEDIDO);
				free_struct_mensaje(rta_cp_comanda->parametros, rta_cp_comanda->tipo_mensaje);
				free(rta_cp_comanda);
				pthread_mutex_unlock(cliente_confirmante->mutex);
				return;
			}
			switch (rta_cp_comanda->tipo_mensaje) {
			case ERROR:
				log_debug(logger_mensajes, "[MENSJS] Comanda respondio ERROR -> FAIL a cli");
				responder_confirm(socket_cliente, false, RTA_CONFIRMAR_PEDIDO);
				free_struct_mensaje(pedido_comanda, CONFIRMAR_PEDIDO);
				free_struct_mensaje(rta_cp_comanda->parametros, rta_cp_comanda->tipo_mensaje);
				free(rta_cp_comanda);
				pthread_mutex_unlock(cliente_confirmante->mutex);
				return;
			case RTA_CONFIRMAR_PEDIDO:
				if (!*(int*)(rta_cp_comanda->parametros)) {
					log_debug(logger_mensajes, "[MENSJS] Comanda respondio FAIL -> FAIL a cli");
					responder_confirm(socket_cliente, false, RTA_CONFIRMAR_PEDIDO);
					free_struct_mensaje(pedido_comanda, CONFIRMAR_PEDIDO);
					free_struct_mensaje(rta_cp_comanda->parametros, rta_cp_comanda->tipo_mensaje);
					free(rta_cp_comanda);
					pthread_mutex_unlock(cliente_confirmante->mutex);
					return;
				}
				log_debug(logger_mensajes, "[MENSJS] Comanda respondio OK");
				planif_nuevoPedido(cliente_id, pedido_comanda->id);
				responder_confirm(socket_cliente, true, RTA_CONFIRMAR_PEDIDO);
				free_struct_mensaje(pedido, CONFIRMAR_PEDIDO);
				free_struct_mensaje(rta_cp_comanda->parametros, rta_cp_comanda->tipo_mensaje);
				free(rta_cp_comanda);
				pthread_mutex_unlock(cliente_confirmante->mutex);
				return;
			default:
				log_debug(logger_mensajes, "[MENSJS] Comanda no respondio lo esperado -> FAIL a cli");
				responder_confirm(socket_cliente, false, RTA_CONFIRMAR_PEDIDO);
				free_struct_mensaje(pedido, CONFIRMAR_PEDIDO);
				free_struct_mensaje(rta_cp_comanda->parametros, rta_cp_comanda->tipo_mensaje);
				free(rta_cp_comanda);
				pthread_mutex_unlock(cliente_confirmante->mutex);
				return;
			}
			// break;
			default:
				log_debug(logger_mensajes, "[MENSJS] Rest. no respondio lo esperado -> FAIL a cli");
				responder_confirm(socket_cliente, false, RTA_CONFIRMAR_PEDIDO);
				qr_free_form(form);
				free_struct_mensaje(pedido, CONFIRMAR_PEDIDO);
				pthread_mutex_unlock(cliente_confirmante->mutex);
				return;
		}
		// break;
		default:
			log_debug(logger_mensajes, "[MENSJS] Comanda no respondio lo esperado -> FAIL a cli");
			responder_confirm(socket_cliente, false, RTA_CONFIRMAR_PEDIDO);
			free_struct_mensaje(rta_obtener_pedido->parametros, rta_obtener_pedido->tipo_mensaje);
			free(rta_obtener_pedido);
			pthread_mutex_unlock(cliente_confirmante->mutex);
			return;
	}
}

void gestionar_CONSULTAR_PEDIDO(uint32_t* id_pedido, int socket_cliente, int cliente_id) {
	t_mensaje* rta_OPcom;
	t_mensaje* rta_final_cli;
	t_restaurante* rest_en_cuestion;
	t_cliente* cliente_en_cuestion;
	t_nombre_y_id pedido_params;
	rta_consultarPedido* rta_a_cliente_params;
	rta_obtenerPedido* params_rta_OPcom;

	if (modo_noComanda) {
		log_debug(logger_mensajes, "[MENSJS]: Modo noComanda, no hay consultar pedido");
		responder_ERROR(socket_cliente);
		return;
	}

	log_debug(logger_mensajes, "[MENSJS]: Se consulta el pedido %i", *id_pedido);

	cliente_en_cuestion = get_cliente_porSuID(cliente_id);
	rest_en_cuestion = cliente_en_cuestion->restaurante_seleccionado;

	pedido_params.id = *id_pedido;
	pedido_params.nombre.nombre = string_duplicate(rest_en_cuestion->nombre);
	rta_OPcom = mensajear_comanda(OBTENER_PEDIDO, &pedido_params, false);

	if (!rta_OPcom) {
		log_debug(logger_mensajes, "[MENSJS]: Hubo error al recibir respuesta de comanda");
		responder_ERROR(socket_cliente);
		return;
	}
	switch (rta_OPcom->tipo_mensaje) {
	case ERROR:
		log_debug(logger_mensajes, "[MENSJS]: Se recibio ERROR de comanda");
		responder_ERROR(socket_cliente);
		free(rta_OPcom);
		return;
	case RTA_OBTENER_PEDIDO:
		log_debug(logger_mensajes, "[MENSJS]: La respuesta de comanda fue la esperada");
		params_rta_OPcom = rta_OPcom->parametros;
		rta_a_cliente_params = malloc(sizeof(rta_consultarPedido));
		rta_a_cliente_params->restaurante.nombre = rest_en_cuestion->nombre;
		rta_a_cliente_params->platos = params_rta_OPcom->infoPedidos; // TODO: hmmmmm
		rta_a_cliente_params->estadoPedido = params_rta_OPcom->estadoPedido;
		rta_final_cli = malloc(sizeof(t_mensaje));
		rta_final_cli->id = cfval_id;
		rta_final_cli->tipo_mensaje = RTA_CONSULTAR_PEDIDO;
		rta_final_cli->parametros = rta_a_cliente_params;
		enviar_mensaje(rta_final_cli, socket_cliente);
		free(rta_a_cliente_params);
		free(rta_final_cli);
		free_struct_mensaje(rta_OPcom->parametros, rta_OPcom->tipo_mensaje);
		free(rta_OPcom);
		break;
	default:
		log_debug(logger_mensajes, "[MENSJS]: La respuesta de comanda no fue la esperada");
		responder_ERROR(socket_cliente);
		free_struct_mensaje(rta_OPcom->parametros, rta_OPcom->tipo_mensaje);
		free(rta_OPcom);
		return;
	}
}

void gestionar_PLATO_LISTO(m_platoListo* plato_params, int socket_rest) {
	t_mensaje* mje_rtaPL;
	t_mensaje* mje_rtaOP;
	t_nombre_y_id* pedido_params;
	t_pedido* pedido;

	// mandar plato listo a comanda
	mje_rtaPL = mensajear_comanda(PLATO_LISTO, plato_params, false);
	if (!mje_rtaPL) {
		responder_ERROR(socket_rest);
		free_struct_mensaje(plato_params, PLATO_LISTO);
		return;
	}
	switch (mje_rtaPL->tipo_mensaje) {
	case ERROR:
		responder_ERROR(socket_rest);
		free_struct_mensaje(plato_params, PLATO_LISTO);
		free_struct_mensaje(mje_rtaPL->parametros, mje_rtaPL->tipo_mensaje);
		free(mje_rtaPL);
		return;
	case RTA_PLATO_LISTO:
		if (!*(int*)(mje_rtaPL->parametros)) {
			responder_ERROR(socket_rest);
			free_struct_mensaje(plato_params, PLATO_LISTO);
			free_struct_mensaje(mje_rtaPL->parametros, mje_rtaPL->tipo_mensaje);
			free(mje_rtaPL);
			return;
		}
		// mandar obtener pedido a comanda
		free_struct_mensaje(mje_rtaPL->parametros, mje_rtaPL->tipo_mensaje);
		free(mje_rtaPL);
		pedido_params = malloc(sizeof(t_nombre_y_id));
		pedido_params->id = plato_params->idPedido;
		pedido_params->nombre.nombre = string_duplicate(plato_params->restaurante.nombre);
		mje_rtaOP = mensajear_comanda(OBTENER_PEDIDO, pedido_params, true);
		if (!mje_rtaOP) {
			log_debug(logger_mensajes, "[MENSJS] Hubo un error al recibir la respuesta de comanda");
			responder_ERROR(socket_rest);
			free_struct_mensaje(plato_params, PLATO_LISTO);
			return;
		}
		switch (mje_rtaOP->tipo_mensaje) {
		case ERROR:
			log_debug(logger_mensajes, "[MENSJS] Comanda respondio ERROR");
			responder_ERROR(socket_rest);
			free_struct_mensaje(plato_params, PLATO_LISTO);
			free(mje_rtaOP);
			return;
		case RTA_OBTENER_PEDIDO:
			log_debug(logger_mensajes, "[MENSJS] La respuesta de comanda es la esperada");
			if (todosLosPlatosEstanPreparados(mje_rtaOP->parametros)) {
				log_debug(logger_mensajes, "[MENSJS] Todos los platos estan preparados");
				pedido = get_pedido(plato_params->idPedido, plato_params->restaurante.nombre, false);
				if (!pedido) {
					log_debug(logger_mensajes, "[MENSJS] No se encontro el pedido %i del rest. %s",
							plato_params->idPedido,
							plato_params->restaurante.nombre
					);
					responder_ERROR(socket_rest);
					free_struct_mensaje(plato_params, PLATO_LISTO);
					free(mje_rtaOP);
					return;
				}
				log_debug(logger_mensajes, "[MENSJS] Se encontro el pedido %i del rest. %s",
						pedido->pedido_id,
						pedido->restaurante->nombre
				);
				pthread_mutex_unlock(pedido->estaPreparado);
				// TODO: ok al restaurant
			} else {
				log_debug(logger_mensajes, "[MENSJS] El pedido no esta completo todavia");
			}
		default:
			responder_ERROR(socket_rest);
			free_struct_mensaje(plato_params, PLATO_LISTO);
			free_struct_mensaje(mje_rtaOP->parametros, mje_rtaOP->tipo_mensaje); // TODO: ojo mje vacio?
					free(mje_rtaOP);
					return;
		}

		default:
			responder_ERROR(socket_rest);
			free_struct_mensaje(plato_params, PLATO_LISTO);
			free_struct_mensaje(mje_rtaPL->parametros, mje_rtaPL->tipo_mensaje);
			free(mje_rtaPL);
			return;
	}
}

bool todosLosPlatosEstanPreparados(rta_obtenerPedido* pedido) {
	bool todosLosPlatosEstanPreparados = true;
	t_elemPedido* plato;
	for (
			unsigned index = 0;
			index < list_size(pedido->infoPedidos);
			index++
	) {
		plato = (t_elemPedido*)list_get(pedido->infoPedidos, index);
		if (plato->cantHecha < plato->cantTotal) {
			todosLosPlatosEstanPreparados = false;
			break;
		}
	}
	return todosLosPlatosEstanPreparados;
}

t_pedido* get_pedido(int id_pedido, char* nombre_restaurante, bool mutex_pedidos_locked_outside) {
	t_pedido* pedido_aux = NULL;
	t_pedido* pedido_return = NULL;
	if (!mutex_pedidos_locked_outside) pthread_mutex_lock(&mutex_pedidos);
	for (
			unsigned index = 0;
			index < list_size(pedidos);
			index++
	) {
		pedido_aux = (t_pedido*)list_get(pedidos, index);
		log_debug(
				logger_mensajes,
				"[MENSJS] \t%3i: Pedido %i, rest. %s",
				index,
				pedido_aux->pedido_id,
				pedido_aux->restaurante->nombre
		);
		if (
				(pedido_aux->pedido_id == id_pedido)
				&& (string_equals_ignore_case(pedido_aux->restaurante->nombre, nombre_restaurante))
		) {
			pedido_return = pedido_aux;
			log_debug(logger_mensajes, "[MENSJS] \t\tEncontrado!");
			break;
		}
	}
	if (!mutex_pedidos_locked_outside) pthread_mutex_unlock(&mutex_pedidos);
	return pedido_return;
}

t_mensaje* mensajear_comanda(op_code cod_op_env, void* params, bool liberar_params) {
	t_mensaje* mensaje_env;
	t_mensaje* mensaje_rec;
	void* stream;
	int socket;
	int cod_op_rec;
	uint32_t id_recibida;
	int _recv_op;
	int _recv_id;
	int error = 0;

	// conex
	socket = iniciar_cliente(cfval_ipComanda, cfval_puertoComanda);
	if (socket == -1) {
		// TODO: error handling
		return NULL;
	}

	log_debug(logger_mensajes, "[MENSJS] (en mensajear_comanda) cliente establecido contra comanda (socket %i)", socket);

	// enviar
	mensaje_env = malloc(sizeof(t_mensaje));
	mensaje_env->tipo_mensaje = cod_op_env;
	mensaje_env->id = 0;
	mensaje_env->parametros = params;
	log_debug(logger_mensajes, "[MENSJS] (en mensajear_comanda) pre envio");
	enviar_mensaje(mensaje_env, socket);
	log_debug(logger_mensajes, "[MENSJS] (en mensajear_comanda) post envio");
	if (liberar_params) {
		free_struct_mensaje(mensaje_env->parametros, mensaje_env->tipo_mensaje);
	}
	free(mensaje_env);

	// recibir rta
	log_debug(logger_mensajes, "[MENSJS] (en mensajear_comanda) pre recv cod_op, socket: %i", socket);
	_recv_op = recv(socket, &cod_op_rec, sizeof(op_code), MSG_WAITALL);
	if (_recv_op == -1 && _recv_op == 0) {
		perror("");
		// TODO: error handling
		return NULL;
	}
	log_debug(logger_mensajes, "[MENSJS] (en mensajear_comanda) post recv cod_op");

	log_debug(logger_mensajes, "[MENSJS] (en mensajear_comanda) pre recv id");
	_recv_id = recv(socket, &id_recibida, sizeof(uint32_t), MSG_WAITALL);
	if (_recv_id == -1 && _recv_id == 0) {
		// TODO: error handling
		return NULL;
	}
	log_debug(logger_mensajes, "[MENSJS] (en mensajear_comanda) post recv id");

	mensaje_rec = malloc(sizeof(t_mensaje));
	log_debug(logger_mensajes, "[MENSJS] (en mensajear_comanda) rta_comanda: %x", mensaje_rec);
	mensaje_rec->tipo_mensaje = cod_op_rec;
	mensaje_rec->id = id_recibida;
	stream = recibir_mensaje(socket, &error);
	if (error) {
		log_debug(logger_mensajes, "[MENSJS] (en mensajear_comanda) entro en if para liberar mensaje_rec");
		free(mensaje_rec);
		return NULL;
	}
	log_debug(logger_mensajes, "[MENSJS] (en mensajear_comanda, despues del if) rta_comanda: %x", mensaje_rec);
	mensaje_rec->parametros = deserializar_mensaje(stream, cod_op_rec);
	// switch (cod_op_rec) {
	//     case RTA_CONSULTAR_PLATOS: ;
	//     case RTA_CREAR_PEDIDO: ;
	//     case RTA_GUARDAR_PEDIDO: ;
	//     case RTA_GUARDAR_PLATO: ;
	//     case RTA_OBTENER_PEDIDO: ;
	//     case RTA_CONFIRMAR_PEDIDO: ;
	//     case RTA_PLATO_LISTO: ;
	//     case RTA_FINALIZAR_PEDIDO: ;
	// }
	return mensaje_rec;
}

void responder_ERROR(int socket) {
	t_mensaje* mensaje = malloc(sizeof(t_mensaje));
	mensaje->tipo_mensaje=ERROR;
	mensaje->id = cfval_id;
	enviar_mensaje(mensaje, socket);
	log_debug(
			logger_mensajes,
			"[MENSJS]: ERROR enviado"
	);
	free(mensaje);
}























void qr_free_form(qr_form_t* form) {
	if (!*(form->error_flag)) {
		free_struct_mensaje(form->m_recibir->parametros, form->m_recibir->tipo_mensaje);
		free(form->m_recibir);
	}
	free_struct_mensaje(form->m_enviar->parametros, form->m_enviar->tipo_mensaje);
	free(form->m_enviar);
	pthread_mutex_destroy(form->mutex);
	free(form->error_flag);
	free(form);
}

qr_form_t* qr_request(t_mensaje* m_enviar, t_restaurante* rest) { // TODO: y si le paso char* nombre_rest?
	qr_form_t* form = malloc(sizeof(qr_form_t));
	form->m_enviar = m_enviar;
	form->m_enviar->id = 0;
	form->m_recibir = NULL;
	form->mutex = malloc(sizeof(pthread_mutex_t));
	form->error_flag = malloc(sizeof(error_flag_t));
	*(form->error_flag) = FLAG_OK;
	pthread_mutex_init(form->mutex, NULL);

	pthread_mutex_lock(rest->q_mtx);
	list_add(rest->q, form);
	sem_post(rest->q_sem);
	pthread_mutex_unlock(rest->q_mtx);
	pthread_mutex_lock(form->mutex);
	pthread_mutex_lock(form->mutex);
	return form;
}

void* qr_admin(t_restaurante* rest) { // pthread_create(rest->q_admin, NULL, qr_admin, rest);
	qr_form_t* form;
	op_code cod_op;
	uint32_t id_recibida;
	int size = 0;
	int _recv_op, _recv_id;
	void* stream;
	int error;

	log_debug(logger_mensajes, "[Q (\"%s\")] Comenzando hilo...", rest->nombre);

	while (1) {
		log_debug(logger_mensajes, "[Q (\"%s\")] Esperando forms...", rest->nombre);
		sem_wait(rest->q_sem);
		log_debug(logger_mensajes, "[Q (\"%s\")] Hay form(s); lockeando mutex de la q...", rest->nombre);
		pthread_mutex_lock(rest->q_mtx);
		log_debug(logger_mensajes, "[Q (\"%s\")] Mutex de la q lockeado", rest->nombre);
		form = list_remove(rest->q, 0);
		pthread_mutex_unlock(rest->q_mtx);
		log_debug(logger_mensajes, "[Q (\"%s\")] Mutex de la q unlockeado", rest->nombre);

		log_debug(logger_mensajes, "[Q (\"%s\")] Enviando mensaje...", rest->nombre);

		loggear_mensaje_enviado(form->m_enviar->parametros, form->m_enviar->tipo_mensaje, logger_mensajes);
		log_debug(logger_mensajes, "[Q (\"%s\")] Socket rest.: %i", rest->nombre, rest->socket);

		enviar_mensaje(form->m_enviar, rest->socket);
		log_debug(logger_mensajes, "[Q (\"%s\")] Mensaje enviado", rest->nombre);
		// TODO: frees?

		log_debug(logger_mensajes, "[Q (\"%s\")] Recibiendo mensaje: (1/3) op_code", rest->nombre);
		_recv_op = recv(rest->socket, &cod_op, sizeof(op_code), MSG_WAITALL);
		if (_recv_op == -1 || _recv_op == 0) {
			log_debug(logger_mensajes, "[Q (\"%s\")] Error al recibir op_code", rest->nombre);
			// TODO: logging
			*(form->error_flag) = FLAG_ERROR;
			log_debug(logger_mensajes, "[Q (\"%s\")] Unlockeando mutex del hilo interesado...", rest->nombre);
			pthread_mutex_unlock(form->mutex);
			log_debug(logger_mensajes, "[Q (\"%s\")] Mutex del hilo interesado unlockeado; recomenzando ciclo", rest->nombre);
			continue;
		}
		log_debug(logger_mensajes, "[Q (\"%s\")] Codigo de operacion %s", rest->nombre, op_code_to_string(cod_op));

		log_debug(logger_mensajes, "[Q (\"%s\")] Recibiendo mensaje: (2/3) id", rest->nombre);
		_recv_id = recv(rest->socket, &id_recibida, sizeof(uint32_t), MSG_WAITALL);
		if (_recv_id == -1 || _recv_id == 0) {
			log_debug(logger_mensajes, "[Q (\"%s\")] Error al recibir id", rest->nombre);
			// TODO: logging
			*(form->error_flag) = FLAG_ERROR;
			log_debug(logger_mensajes, "[Q (\"%s\")] Unlockeando mutex del hilo interesado...", rest->nombre);
			pthread_mutex_unlock(form->mutex);
			log_debug(logger_mensajes, "[Q (\"%s\")] Mutex del hilo interesado unlockeado; recomenzando ciclo", rest->nombre);
			continue;
		}
		log_debug(logger_mensajes, "[Q (\"%s\")] ID recibida: %i", rest->nombre, id_recibida);

		form->m_recibir = malloc(sizeof(t_mensaje));
		form->m_recibir->tipo_mensaje = cod_op;
		form->m_recibir->id = id_recibida;

		log_debug(logger_mensajes, "[Q (\"%s\")] Recibiendo mensaje: (3/3) parametros", rest->nombre);

		stream = recibir_mensaje(rest->socket, &error);

		if (error) {
			log_debug(logger_mensajes, "[Q (\"%s\")] Hubo un error en la recepcion del stream", rest->nombre);
			*(form->error_flag) = FLAG_ERROR;
			form->m_recibir->parametros = NULL;
		} else {
			form->m_recibir->parametros = deserializar_mensaje(
					stream,
					cod_op
			);
			loggear_mensaje_recibido(form->m_recibir->parametros, cod_op, logger_mensajes);
		}


		log_debug(logger_mensajes, "[Q (\"%s\")] Unlockeando mutex del hilo interesado...", rest->nombre);
		pthread_mutex_unlock(form->mutex);
		log_debug(logger_mensajes, "[Q (\"%s\")] Mutex del hilo interesado unlockeado; recomenzando ciclo", rest->nombre);
	}
}
