#include "App.h"

int main(int argc, char* argv[]) {

	// modos de debug
		modo_noComanda = true;
		modo_noRest = true;
		modo_mock = true;

	// configuracion situacional de loggers
		logger_obligatorio_consolaActiva = false;
		
		logger_configuracion_consolaActiva = true;
		logger_configuracion_path = "./configuracion.log";

		logger_planificacion_consolaActiva = true;
		logger_planificacion_path = "./planificacion.log";

		logger_mensajes_consolaActiva = true;
		logger_mensajes_path = "./mensajes.log";
	
	// lectura de config y configuracion inicial y de conexiones
		configuracionInicial();
		configuracionConexiones();

	// inicializacion de hilos de planificacion
		pthread_create(&hilo_planificador_cortoPlazo, NULL, fhilo_planificador_cortoPlazo, NULL);
		pthread_create(&hilo_planificador_largoPlazo, NULL, fhilo_planificador_largoPlazo, NULL);
		pthread_create(&hilo_clock, NULL, fhilo_clock, NULL);
	
	// Al momento de inicializar el proceso se realizarán las siguientes operaciones:
		
		// Se conectará al módulo Comanda.
			
			// if (modo_noComanda) {
			// 	log_debug(logger_mensajes, "[MAIN] Modo no-Comanda: salteando conexion a Comanda");
			// } else {
			// 	log_debug(logger_mensajes, "[MAIN] Lanzando hilo de conexion a Comanda...");
			// 	pthread_create(&hilo_conectarConComanda, NULL, fhilo_conectarConComanda, NULL);
			// }
		
		// Abrirá un puerto de escucha para que los distintos Restaurantes se puedan conectar a él.	
		log_debug(logger_mensajes, "[MAIN] Modo no-Comanda: Lanzando hilo de server...");
		pthread_create(&hilo_servidor, NULL, fhilo_servidor, NULL);
		

	// if (modo_noComanda) {
	// 	log_debug(logger_mensajes, "[MAIN] Modo no-Comanda: salteando join de hilo de conexion a Comanda");
	// } else {
	// 	log_debug(logger_mensajes, "[MAIN] Joineando hilo de conexion a Comanda...");
	// 	pthread_join(hilo_conectarConComanda, NULL);
	// }

	if (modo_mock) {
		mock_mensajes();
	}

	log_debug(logger_mensajes, "[MAIN] Esperando join hilo de server...");
	pthread_join(hilo_servidor, NULL);
	log_debug(logger_mensajes, "[MAIN] Hilo de server joineado");

	liberar_memoria();

	return EXIT_SUCCESS;
}

void mock_mensajes(void) {
	t_restaurante* restaurante = malloc(sizeof(t_restaurante));
    restaurante->nombre = "MockDonalds";
    restaurante->pos_x = 5;
    restaurante->pos_y = 4;
    restaurante->socket = 0;
    restaurante->mutex = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(restaurante->mutex, NULL);
    pthread_mutex_lock(&mutex_lista_restaurantes);
    list_add(restaurantes, restaurante);
    // TODO: logging
    pthread_mutex_unlock(&mutex_lista_restaurantes);

	t_cliente* cliente = malloc(sizeof(t_cliente));
    cliente->id = 77;
    cliente->pos_x = 10;
    cliente->pos_y = 11;
	cliente->pedido_id = 808;
    cliente->restaurante_seleccionado = restaurante;
    cliente->socket = 0;
    cliente->mutex = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(cliente->mutex, NULL);
    pthread_mutex_lock(cliente->mutex);

    pthread_mutex_lock(&mutex_lista_clientes);
    list_add(clientes, cliente);
    // TODO: logging
    pthread_mutex_unlock(&mutex_lista_clientes);

	planif_nuevoPedido(808);

	return;
}