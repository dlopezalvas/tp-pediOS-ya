#include "SindicatoApi.h"

/* ********************************** PRIVATE FUNCTIONS ********************************** */

void internal_api_create_folder(char* path){
	struct stat st = {0};

	/* Validate if the folder exists to create the folder */
	if(stat(path, &st) == -1){
		if(mkdir(path,0777) == 0){
			log_info(sindicatoDebugLog, "[FILESYSTEM] Carpeta creada: %s",path);
		}
	}else{
		log_info(sindicatoDebugLog, "[FILESYSTEM] Carpeta existente: %s", path);
	}
}

char* internal_api_build_path(char* path, char* toAppend){
	char* pathBuilded = string_duplicate(path);
	string_append(&pathBuilded, toAppend);
	return pathBuilded;
}

/* ********************************** PUBLIC  FUNCTIONS ********************************** */

/* Console functions */
void sindicato_api_crear_restaurante(char* nombre, int cantCocineros, int* posXY, char** afinidadCocinero, char** platos, int* precioPlatos, int cantHornos){
	printf("Creo el restaurante\n");
	printf("%s\n", nombre);
	printf("%s\n", cantCocineros);
	printf("%s\n", posXY);
	printf("%s\n", afinidadCocinero);
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
void sindicato_api_send_response_of_operation(t_responseMessage* response, int socket_cliente){
	loggear_mensaje_enviado(response->message->parametros, response->message->tipo_mensaje, sindicatoLog);
	enviar_mensaje(response->message, socket_cliente);
	//enviar_mensaje(response->message, response->message);

	//ya estoy habilitado para hacer los free();
}

t_restaurante_y_plato* sindicato_api_consultar_platos(void* consultaPatos){
	/* Initialize of pedido structure */
	t_restaurante_y_plato* platos = malloc(sizeof(t_restaurante_y_plato));
	platos->nombres = list_create();
	t_nombre* plato = malloc(sizeof(t_nombre));

	/* DELETE THIS: datos dummies solo para TEST */
	plato->nombre = "Milanesa";
	log_info(sindicatoDebugLog, plato->nombre);


	list_add(platos->nombres, plato);
	platos->cantElementos = platos->nombres->elements_count;

	return platos;
}

uint32_t* sindicato_api_guardar_pedido(void* pedido){
	//t_nombre_y_id* asd;

	uint32_t* opResult = malloc(sizeof(uint32_t));
	/* DELETE THIS: datos dummies solo para TEST */
	(*opResult) = 1;

	return opResult;
}

uint32_t* sindicato_api_guardar_plato(void* pedido){
	//m_guardarPlato* asd;

	uint32_t* opResult = malloc(sizeof(uint32_t));
	/* DELETE THIS: datos dummies solo para TEST */
	(*opResult) = 1;

	return opResult;
}

uint32_t* sindicato_api_confirmar_pedido(void* pedido){
	//t_nombre_y_id* asd;
	uint32_t* opResult = malloc(sizeof(uint32_t));
	/* DELETE THIS: datos dummies solo para TEST */
	(*opResult) = 1;

	return opResult;
}

rta_obtenerPedido* sindicato_api_obtener_pedido(void* Consultapedido){
	//t_nombre_y_id* asd;
	rta_obtenerPedido* pedido = malloc(sizeof(rta_obtenerPedido));
	pedido->infoPedidos = list_create();

	t_elemPedido* pedidoElem = malloc(sizeof(t_elemPedido));

	/* DELETE THIS: datos dummies solo para TEST */
	pedidoElem->cantHecha = 1;
	pedidoElem->cantTotal = 1;
	pedidoElem->comida.nombre = "Milanesa";
	log_info(sindicatoDebugLog, pedidoElem->comida.nombre);

	pedido->cantPedidos = 1;
	pedido->estadoPedido = PENDIENTE;
	list_add(pedido->infoPedidos, pedidoElem);

	return pedido;
}

rta_obtenerRestaurante* sindicato_api_obtener_restaurante(void* restaurante){
	/* Initialize of restaurante structure */
	rta_obtenerRestaurante* restauranteInfo = malloc(sizeof(rta_obtenerRestaurante));
	restauranteInfo->afinidades = list_create();
	restauranteInfo->recetas = list_create();

	/* Elements of list */
	t_receta* recetaPrecio = malloc(sizeof(t_receta));
	t_nombre* afinidad = malloc(sizeof(t_nombre));

	/* DELETE THIS: datos dummies solo para TEST */
	recetaPrecio->receta.nombre = "Milanesa";
	recetaPrecio->precio = 500;

	afinidad->nombre = "Empanadas";

	restauranteInfo->cantAfinidades = 1;
	list_add(restauranteInfo->afinidades, afinidad);
	restauranteInfo->posicion.x = 1;
	restauranteInfo->posicion.y = 2;
	restauranteInfo->cantRecetas = 1;
	list_add(restauranteInfo->recetas,recetaPrecio);
	restauranteInfo->cantHornos = 1;
	restauranteInfo->cantCocineros = 2;
	restauranteInfo->cantPedidos = 2;

	return restauranteInfo;
}

uint32_t* sindicato_api_plato_listo(void* plato){
	//m_platoListo* asd;
	uint32_t* opResult = malloc(sizeof(uint32_t));
	/* DELETE THIS: datos dummies solo para TEST */
	(*opResult) = 1;

	return opResult;
}

rta_obtenerReceta* sindicato_api_obtener_receta(void* plato){
	//t_nombre* asd;
	rta_obtenerReceta* receta = malloc(sizeof(rta_obtenerReceta));
	receta->pasos = list_create();

	t_paso* paso = malloc(sizeof(t_paso));
	paso->duracion = 1;
	paso->paso.nombre = "Milanesear";
	puts(paso->paso.nombre);

	/* DELETE THIS: datos dummies solo para TEST */
	list_add(receta->pasos,paso);
	receta->cantPasos = 1;

	return receta;
}

uint32_t* sindicato_api_terminar_pedido(void* pedido){
	//t_nombre_y_id * asd;
	uint32_t* opResult = malloc(sizeof(uint32_t));
	/* DELETE THIS: datos dummies solo para TEST */
	(*opResult) = 1;

	return opResult;
}

/* Main functions */
void sindicato_api_afip_initialize(){

	/* Create mount point of the FS */
	internal_api_create_folder(sindicatoMountPoint);

	/* Create folder "Metadata" in {mount_point} */
	char* sindicatoMetadataPath = internal_api_build_path(sindicatoMountPoint, "/Metadata");
	internal_api_create_folder(sindicatoMetadataPath);
	free(sindicatoMetadataPath);

	/* Create folder "Files" in {mount_point} */
	char* sindicatoFilesPath = internal_api_build_path(sindicatoMountPoint, "/Files");
	internal_api_create_folder(sindicatoFilesPath);
	free(sindicatoFilesPath);

	/* Create folder "Receta" in {mount_point}/Files */
	char* sindicatoRecetaPath = internal_api_build_path(sindicatoMountPoint, "/Files/Receta");
	internal_api_create_folder(sindicatoRecetaPath);
	free(sindicatoRecetaPath);

	/* Create folder "Restaurante" in {mount_point}/Files */
	char* sindicatoRestaurantePath = internal_api_build_path(sindicatoMountPoint, "/Files/Restaurante");
	internal_api_create_folder(sindicatoRestaurantePath);
	free(sindicatoRestaurantePath);
}
