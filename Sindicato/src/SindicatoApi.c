#include "SindicatoApi.h"

/* ********************************** PRIVATE FUNCTIONS ********************************** */


/* ********************************** PUBLIC  FUNCTIONS ********************************** */

/* Console functions */
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

/* Server functions */
void sindicato_api_send_response_of_operation(t_responseMessage response){
	log_info(sindicatoLog, "send response");
	return;
}

t_restaurante_y_plato sindicato_api_consultar_platos(void* message){
	t_nombre* restaurante = message;
	t_restaurante_y_plato res;

	//Validar si existe el restaurante


	return res;

}

int sindicato_api_guardar_pedido(void* pedido){
	t_nombre_y_id* asd;
	return 0;
}

int sindicato_api_guardar_plato(void* pedido){
	m_guardarPlato* asd;
	return 0;
}

int sindicato_api_confirmar_pedido(void* pedido){
	t_nombre_y_id* asd;
	return 0;
}

rta_obtenerPedido sindicato_api_obtener_pedido(void* pedido){
	t_nombre_y_id* asd;
	rta_obtenerPedido res;
	return res;
}

rta_obtenerRestaurante sindicato_api_obtener_restaurante(void* restaurante){
	t_nombre* asd;
	rta_obtenerRestaurante res;
	return res;
}

int sindicato_api_plato_listo(void* plato){
	m_platoListo* asd;
	return 0;

}

rta_obtenerReceta sindicato_api_obtener_receta(void* plato){
	t_nombre* asd;
	rta_obtenerReceta res;
	return res;
}

int sindicato_api_terminar_pedido(void* pedido){
	t_nombre_y_id * asd;
	return 0;
}

/* Main functions */
void sindicato_api_afip_initialize(){
	char* mountPoint = config_get_string_value(sindicatoConfig,"PUNTO_MONTAJE");

	//TODO: montar el FS
}
