#include "SindicatoApi.h"

/* ********************************** PRIVATE FUNCTIONS ********************************** */


/* ********************************** PUBLIC  FUNCTIONS ********************************** */

/* Console functions */
void sindicato_api_crear_restaurante(char* nombre, int cantCocineros, int* posXY, char** afinidades, char** platos, int* precioPlatos, int cantHornos){
	printf("Creo el restaurante\n");
	printf("%s\n", nombre);
	printf("%s\n", cantCocineros);
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
void sindicato_api_send_response_of_operation(t_responseMessage* response){
	loggear_mensaje_enviado(response->message->parametros, response->message->tipo_mensaje, sindicatoLog);
	enviar_mensaje(response->message, response->socket);
}

t_restaurante_y_plato* sindicato_api_consultar_platos(void* consultaPatos){
	/* Initialize of pedido structure */
	t_restaurante_y_plato* platos = malloc(sizeof(t_restaurante_y_plato));
	platos->nombres = list_create();

	/*t_nombre* restaurante = consultaPatos;
	log_info(sindicatoLog, restaurante->nombre);*/

	/* DELETE THIS: datos dummies solo para TEST */
	list_add(platos->nombres, "hola");
	platos->cantElementos = 1;

	return platos;
}

int sindicato_api_guardar_pedido(void* pedido){
	//t_nombre_y_id* asd;
	return 0;
}

int sindicato_api_guardar_plato(void* pedido){
	//m_guardarPlato* asd;
	return 0;
}

int sindicato_api_confirmar_pedido(void* pedido){
	//t_nombre_y_id* asd;
	return 0;
}

rta_obtenerPedido* sindicato_api_obtener_pedido(void* Consultapedido){
	//t_nombre_y_id* asd;
	rta_obtenerPedido* pedido = malloc(sizeof(rta_obtenerPedido));
	pedido->infoPedidos = list_create();

	/* DELETE THIS: datos dummies solo para TEST */
	pedido->cantPedidos = 1;
	list_add(pedido->infoPedidos, "Milanesa");
	pedido->estadoPedido = PENDIENTE;

	return pedido;
}

rta_obtenerRestaurante* sindicato_api_obtener_restaurante(void* restaurante){
	/* Initialize of restaurante structure */
	rta_obtenerRestaurante* restauranteInfo = malloc(sizeof(rta_obtenerRestaurante));
	restauranteInfo->cocineroAfinidad = list_create();
	restauranteInfo->recetas = list_create();

	t_list* afinidadPrecio = list_create();
	list_add(afinidadPrecio, "Milanesa");
	list_add(afinidadPrecio, "200");

	/* DELETE THIS: datos dummies solo para TEST */
	restauranteInfo->cantCocineroAfinidad = 1;
	restauranteInfo->cantRecetas = 1;
	restauranteInfo->cantHornos = 1;
	restauranteInfo->posicion.x = 1;
	restauranteInfo->posicion.y = 2;
	list_add(restauranteInfo->cocineroAfinidad, "Milanesa");
	list_add(restauranteInfo->recetas, afinidadPrecio);

	return restaurante;
}

int sindicato_api_plato_listo(void* plato){
	//m_platoListo* asd;
	return 0;
}

rta_obtenerReceta* sindicato_api_obtener_receta(void* plato){
	rta_obtenerReceta* receta = malloc(sizeof(rta_obtenerReceta));
	receta->pasos = list_create();

	//t_nombre* asd;

	/* DELETE THIS: datos dummies solo para TEST */
	list_add(receta->pasos,"Milanesa");
	receta->cantPasos = 1;

	return receta;
}

int sindicato_api_terminar_pedido(void* pedido){
	//t_nombre_y_id * asd;
	return 0;
}

/* Main functions */
void sindicato_api_afip_initialize(){
	//char* mountPoint = config_get_string_value(sindicatoConfig,"PUNTO_MONTAJE");

	//TODO: montar el FS
}
