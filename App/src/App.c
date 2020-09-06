#include "App.h"

int main(int argc, char* argv[]) {

	// configuracion situacional de loggers
		logger_obligatorio_consolaActiva = false;
		
		logger_configuracion_consolaActiva = true;
		logger_configuracion_path = "../logs_debug/configuracion.log";

	configuracionInicial();
	
	// Al momento de inicializar el proceso se realizarán las siguientes operaciones:

		// Se conectará al módulo Comanda.
		pthread_create(&, NULL, fhilo_conectarConComanda, NULL);

		// Abrirá un puerto de escucha para que los distintos Restaurantes se puedan conectar a él.	
		pthread_create(&, NULL, fhilo_servidor, NULL);

	liberar_memoria();

	return EXIT_SUCCESS;
}
