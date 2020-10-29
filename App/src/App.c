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
	t_restaurante* barDeMoe = mock_registrar_restaurante("El Bar de Moe", 2, 2);
	mock_registrar_cliente(66, 10, 5, 606, barDeMoe);
	t_restaurante* mockDonalds = mock_registrar_restaurante("MockDonalds", 5, 10);
	mock_registrar_cliente(77, 2, 5, 707, mockDonalds);
	t_restaurante* krabbyKrab = mock_registrar_restaurante("El Crustaceo Cascarudo", 9, 4);
	mock_registrar_cliente(88, 5, 5, 808, krabbyKrab);
	t_restaurante* centralPerk = mock_registrar_restaurante("Central Perk", 11, 9);
	mock_registrar_cliente(99, 1, 2, 909, centralPerk);

	mock_confirmar_pedido(66); // TODO: esto no es univoco
	sleep(1);
	mock_confirmar_pedido(77);
	sleep(1);
	mock_confirmar_pedido(88);
	sleep(4);
	mock_confirmar_pedido(99);

	return;
}

t_restaurante* mock_registrar_restaurante(char* nombre, int x, int y) {
	log_debug(logger_planificacion, "[MOCKER] Simulando %s...", nombre);
	t_restaurante* restaurante = malloc(sizeof(t_restaurante));
    restaurante->nombre = nombre;
    restaurante->pos_x = x;
    restaurante->pos_y = y;
    // restaurante->socket = 0;
    restaurante->mutex = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(restaurante->mutex, NULL);
    pthread_mutex_lock(&mutex_lista_restaurantes);
    list_add(restaurantes, restaurante);
    // TODO: logging
    pthread_mutex_unlock(&mutex_lista_restaurantes);

	return restaurante;
}

void mock_registrar_cliente(
		unsigned id,
		int x,
		int y,
		int pedido_id,
		t_restaurante* restaurante_seleccionado
) {
	log_debug(logger_planificacion, "[MOCKER] Simulando cliente %i...", id);
	t_cliente* cliente = malloc(sizeof(t_cliente));
    cliente->id = id;
    cliente->pos_x = x;
    cliente->pos_y = y;
	cliente->pedido_id = pedido_id;
    cliente->restaurante_seleccionado = restaurante_seleccionado;
    // cliente->socket = 0;
    cliente->mutex = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(cliente->mutex, NULL);
    pthread_mutex_lock(cliente->mutex);

    pthread_mutex_lock(&mutex_lista_clientes);
	log_debug(logger_planificacion, "[MOCKER] Agregando cliente %i a la lista...", id);
    list_add(clientes, cliente);
    // TODO: logging
    pthread_mutex_unlock(&mutex_lista_clientes);
}

void mock_confirmar_pedido(int id_cliente) {
	log_debug(logger_planificacion, "[MOCKER] Disparando planif. nuevo pedido (%i)", id_cliente);
	planif_nuevoPedido(id_cliente);
}