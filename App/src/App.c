#include "App.h"

int main(int argc, char* argv[]) {

	// modos de debug
		modo_noComanda = true;
		modo_noRest = true;

	// configuracion situacional de loggers
		logger_obligatorio_consolaActiva = false;
		
		logger_configuracion_consolaActiva = true;
		logger_configuracion_path = "./configuracion.log";

		logger_planificacion_consolaActiva = true;
		logger_planificacion_path = "./planificacion.log";

		logger_mensajes_consolaActiva = true;
		logger_mensajes_path = "./mensajes.log";
	
	configuracionInicial();
	
	configuracionConexiones();
	
	// Al momento de inicializar el proceso se realizarán las siguientes operaciones:
		
		// Se conectará al módulo Comanda.
		if (modo_noComanda) {
			log_debug(logger_mensajes, "[MAIN] Modo no-Comanda: salteando conexion a Comanda");
		} else {
			log_debug(logger_mensajes, "[MAIN] Lanzando hilo de conexion a Comanda...");
			pthread_create(&hilo_conectarConComanda, NULL, fhilo_conectarConComanda, NULL);
		}
		
		// Abrirá un puerto de escucha para que los distintos Restaurantes se puedan conectar a él.	
		log_debug(logger_mensajes, "[MAIN] Modo no-Comanda: Lanzando hilo de server...");
		pthread_create(&hilo_servidor, NULL, fhilo_servidor, NULL);
		

	if (modo_noComanda) {
		log_debug(logger_mensajes, "[MAIN] Modo no-Comanda: salteando join de hilo de conexion a Comanda");
	} else {
		log_debug(logger_mensajes, "[MAIN] Joineando hilo de conexion a Comanda...");
		pthread_join(hilo_conectarConComanda, NULL);
	}

	log_debug(logger_mensajes, "[MAIN] Joineando hilo de server...");
	pthread_join(hilo_servidor, NULL);

	liberar_memoria();

	return EXIT_SUCCESS;
}
