#include "App.h"

int main(int argc, char* argv[]) {

	// configuracion situacional de loggers
		logger_obligatorio_consolaActiva = false;
		
		logger_configuracion_consolaActiva = true;
		logger_configuracion_path = "../logs_debug/configuracion.log";

	configuracionInicial();
	
	// Al momento de inicializar el proceso se realizarán las siguientes operaciones:

		// Se conectará al módulo Comanda.
		socket_cliente_comanda = iniciar_cliente(cfval_ipComanda, cfval_puertoComanda);

		// Abrirá un puerto de escucha para que los distintos Restaurantes se puedan conectar a él.	
		socket_escucha = iniciar_servidor("se aprueba papa", cfval_puertoEscucha);

	liberar_memoria();

	return EXIT_SUCCESS;
}
