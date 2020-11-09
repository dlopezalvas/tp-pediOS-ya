#include "SindicatoApi.h"

/* ********************************** PRIVATE FUNCTIONS ********************************** */

void internal_api_free_array(char** array,int qtyBitsReserved){

	for(int position = 0; position < (qtyBitsReserved - 1); position++){
		free(array[position]);
	}

	free(array);
}

void internal_api_createFileSystemFolders(){
	/* Create mount point of the FS */
	sindicato_utils_create_folder(sindicatoMountPoint, false);

	/* Create folder "Metadata" in {mount_point} */
	char* sindicatoMetadataPath = sindicato_utils_build_path(sindicatoMountPoint, "/Metadata");
	sindicato_utils_create_folder(sindicatoMetadataPath, false);
	free(sindicatoMetadataPath);

	/* Create folder "Files" in {mount_point} */
	char* sindicatoFilesPath = sindicato_utils_build_path(sindicatoMountPoint, "/Files");
	sindicato_utils_create_folder(sindicatoFilesPath, false);
	free(sindicatoFilesPath);

	/* Create folder "Blocks" in {mount_point} */
	sindicatoBlocksPath = sindicato_utils_build_path(sindicatoMountPoint, "/Blocks");
	sindicato_utils_create_folder(sindicatoBlocksPath, false);

	/* Create folder "Receta" in {mount_point}/Files */
	char* sindicatoRecetaPath = sindicato_utils_build_path(sindicatoMountPoint, "/Files/Recetas");
	sindicato_utils_create_folder(sindicatoRecetaPath, false);
	free(sindicatoRecetaPath);

	/* Create folder "Restaurante" in {mount_point}/Files */
	char* sindicatoRestaurantePath = sindicato_utils_build_path(sindicatoMountPoint, "/Files/Restaurantes");
	sindicato_utils_create_folder(sindicatoRestaurantePath, false);
	free(sindicatoRestaurantePath);
}

void internal_api_initialize_metadata(){
	char* metadaPathFS = sindicato_utils_build_path(sindicatoMountPoint, "/Metadata/Metadata.AFIP");

	metadataConfig = config_create(metadaPathFS);

	metadataFS = malloc(sizeof(t_metadata));

	metadataFS->block_size = (uint32_t)config_get_int_value(metadataConfig, "BLOCK_SIZE");
	metadataFS->blocks = (uint32_t)config_get_int_value(metadataConfig, "BLOCKS");
	metadataFS->magic_number = config_get_string_value(metadataConfig, "MAGIC_NUMBER");

	log_info(sindicatoDebugLog,"[FILESYSTEM] Metadata leida y guardada");

	free(metadaPathFS);
}

void internal_api_bitmap_create(){
	char* bitmapPathFS = sindicato_utils_build_path(sindicatoMountPoint, "/Metadata/Bitmap.bin");

	int blocks = metadataFS->blocks/8;

	int bitarrayFile = open(bitmapPathFS, O_RDWR | O_CREAT, 0700);

	ftruncate(bitarrayFile, blocks);

	char* bitarrayMap = mmap(0, blocks, PROT_WRITE | PROT_READ, MAP_SHARED, bitarrayFile, 0);

	//TODO: ver errores en mapeo

	bitarray = bitarray_create_with_mode(bitarrayMap, blocks, LSB_FIRST);

	msync(bitarray, sizeof(bitarray), MS_SYNC);

	pthread_mutex_init(&bitarray_mtx, NULL);

	log_info(sindicatoDebugLog,"[FILESYSTEM] Bitmap creado");

	close(bitarrayFile);
	free(bitmapPathFS);
}

void internal_api_initialize_blocks(){
	char* filePath;
	FILE* bloqueFS;

	for(int i = 1; i <= (int)metadataFS->blocks; i++){
		filePath = sindicato_utils_build_block_path(i);

		/* Los archivos se abren con  "a" para que no overrideen el valor que tengan guardados
		 * en caso de reconexion */
		bloqueFS = fopen(filePath, "a");

		fclose(bloqueFS);
		free(filePath);
	}

	log_info(sindicatoDebugLog,"[FILESYSTEM] Bloques inicializados");
}

void internal_api_free_bits_reserved(char** bitsList, int positionThatFailed){

	int block = 0;
	int qtyBitsReserved = positionThatFailed - 1;

	for(int i = 0; i < qtyBitsReserved; i++){

		block = atoi(bitsList[i]);

		if(bitarray_test_bit(bitarray, block)){

			/* BEGIN CRITICAL SECTION bitarray_mtx */
			pthread_mutex_lock(&bitarray_mtx);

			bitarray_clean_bit(bitarray, block);
			msync(bitarray, sizeof(bitarray), MS_SYNC);

			pthread_mutex_unlock(&bitarray_mtx);
			/* END   CRITICAL SECTION bitarray_mtx */
		}
	}

	log_info(sindicatoDebugLog, "[FILESYSTEM] %d bloques reservados fueron liberados en bitmap.",qtyBitsReserved);
}

int internal_api_get_free_block(){

	for(int blockNumber = 1; blockNumber <= metadataFS->blocks; blockNumber++){
		if(!bitarray_test_bit(bitarray, blockNumber)){
			/* BEGIN CRITICAL SECTION bitarray_mtx */
			pthread_mutex_lock(&bitarray_mtx);

			bitarray_set_bit(bitarray, blockNumber);
			msync(bitarray, sizeof(bitarray), MS_SYNC);

			pthread_mutex_unlock(&bitarray_mtx);
			/* END   CRITICAL SECTION bitarray_mtx */

			return blockNumber;
		}
	}

	return -1;
}

char** internal_api_get_free_blocks(int blocksNeeded){

	char* stringAux;

	int freeBlock = 0;
	char** blocks = malloc(blocksNeeded*sizeof(int));

	for(int i = 1; i <= blocksNeeded; i++){
		freeBlock = internal_api_get_free_block();
		if(freeBlock == -1){
			log_error(sindicatoDebugLog, "[FILESYSTEM] ERROR: No hay bloques disponibles");
			internal_api_free_bits_reserved(blocks, i);
			internal_api_free_array(blocks, i);
			return NULL;
		}

		stringAux = string_itoa(freeBlock);
		blocks[i-1] = string_new();
		string_append(&blocks[i-1], stringAux);
		free(stringAux);
	}

	log_info(sindicatoDebugLog, "[FILESYSTEM] %d bloques fueron seteados en bitmap.",blocksNeeded);

	return blocks;
}

/* ********************************** PUBLIC  FUNCTIONS ********************************** */

/* Console functions */
void sindicato_api_crear_restaurante(char* nombre, char* cantCocineros, char* posXY, char* afinidadCocinero, char* platos, char* precioPlatos, char* cantHornos){
	log_info(sindicatoLog, "Se creo el restaurante: %s %s %s %s %s %s %s",nombre, cantCocineros, posXY, afinidadCocinero, platos, precioPlatos, cantHornos);
}

void sindicato_api_crear_receta(char* nombre, char* pasos, char* tiempoPasos){
	log_info(sindicatoLog, "Se creo la receta: %s %s %s", nombre, pasos, tiempoPasos);
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
	t_nombre* plato = malloc(sizeof(t_nombre));

	/* DELETE THIS: datos dummies solo para TEST */
	plato->nombre = "Milanesa";

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

	internal_api_createFileSystemFolders();

	internal_api_initialize_metadata();

	internal_api_bitmap_create();

	internal_api_initialize_blocks();

	/* DELETE THIS, Test only*/
	internal_api_get_free_blocks(9);

	internal_api_get_free_blocks(3);
}
