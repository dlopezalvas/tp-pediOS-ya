#include "SindicatoApi.h"

void sindicato_api_crear_restaurante(char* nombre, int cantCocineros, int* posXY, char** afinidades, char** platos, int* precioPlatos, int cantHornos){
	printf("Creo el restaurante\n");
	printf("%s\n", nombre);
	printf("%d\n", cantCocineros);
	printf("%s\n", posXY);
	printf("%s\n", afinidades);
	printf("%s\n", platos);
	printf("%s\n", precioPlatos);
	printf("%s\n", cantHornos);
}

void sindicato_api_crear_receta(char* nombre, char** pasos, int* tiempoPasos){
	printf("Creo la receta\n");
	printf("%s\n", nombre);
	printf("%s\n", pasos);
	printf("%s\n", tiempoPasos);
}

/* ********************************** PUBLIC  FUNCTIONS ********************************** */

void sindicato_afip_initialize(){
	char* mountPoint = config_get_string_value(sindicatoConfig,"PUNTO_MONTAJE");

	//TODO: montar el FS
}
