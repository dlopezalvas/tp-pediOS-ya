#include "App.h"

int main(int argc, char* argv[]) {

	// configuracion situacional de loggers
		logger_obligatorio_consolaActiva = false;
		
		logger_configuracion_consolaActiva = true;
		logger_configuracion_path = "./configuracion.log";

		logger_mensajes_consolaActiva = true;
		logger_mensajes_path = "./mensajes.log";

	configuracionInicial();
	
	configuracionConexiones();

	// Al momento de inicializar el proceso se realizarán las siguientes operaciones:

		// Se conectará al módulo Comanda.
		pthread_create(&hilo_conectarConComanda, NULL, fhilo_conectarConComanda, NULL);

		// Abrirá un puerto de escucha para que los distintos Restaurantes se puedan conectar a él.	
		pthread_create(&hilo_servidor, NULL, fhilo_servidor, NULL);

	pthread_join(hilo_conectarConComanda, NULL);
	pthread_join(hilo_servidor, NULL);

	printf("hola\n");

	liberar_memoria();

	return EXIT_SUCCESS;
}
