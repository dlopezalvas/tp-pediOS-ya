#include "App.h"

int main(int argc, char* argv[]) {

	// configuracion situacional de loggers
		logger_obligatorio_consolaActiva = false;
		
		logger_configuracion_consolaActiva = true;
		logger_configuracion_path = "../logs_debug/configuracion.log";

	configuracionInicial();
	
	// Al momento de inicializar el proceso se realizarán las siguientes operaciones:

		// Se conectará al módulo Comanda.

		// Abrirá un puerto de escucha para que los distintos Restaurantes se puedan conectar a él.	

	return EXIT_SUCCESS;
}
