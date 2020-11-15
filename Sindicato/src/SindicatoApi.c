#include "SindicatoApi.h"

/* ********************************** PRIVATE FUNCTIONS ********************************** */

/* ********** INITIALIZE BASIC COMPONENTS ********** */

/* ***** FS FOLDERS ***** */

void internal_api_createFileSystemFolders(){
	/* Create mount point of the FS */
	sindicato_utils_create_folder(sindicatoMountPoint, true);

	/* Create folder "Metadata" in {mount_point} */
	char* sindicatoMetadataPath = sindicato_utils_build_path(sindicatoMountPoint, "/Metadata");
	sindicato_utils_create_folder(sindicatoMetadataPath, true);
	free(sindicatoMetadataPath);

	/* Create folder "Files" in {mount_point} */
	char* sindicatoFilesPath = sindicato_utils_build_path(sindicatoMountPoint, "/Files");
	sindicato_utils_create_folder(sindicatoFilesPath, true);
	free(sindicatoFilesPath);

	/* Create folder "Blocks" in {mount_point} */
	sindicatoBlocksPath = sindicato_utils_build_path(sindicatoMountPoint, "/Blocks");
	sindicato_utils_create_folder(sindicatoBlocksPath, true);


	/* Create folder "Receta" in {mount_point}/Files */
	sindicatoRecetaPath = sindicato_utils_build_path(sindicatoMountPoint, "/Files/Recetas");
	sindicato_utils_create_folder(sindicatoRecetaPath, true);


	/* Create folder "Restaurante" in {mount_point}/Files */
	sindicatoRestaurantePath = sindicato_utils_build_path(sindicatoMountPoint, "/Files/Restaurantes");
	sindicato_utils_create_folder(sindicatoRestaurantePath, true);
}

/* ***** FS METADATA ***** */

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

/* ***** FS BITMAP ***** */

void internal_api_bitmap_create(){
	char* bitmapPathFS = sindicato_utils_build_path(sindicatoMountPoint, "/Metadata/Bitmap.bin");

	int blocks = metadataFS->blocks/8;
	//TODO: validar multiplo de 8

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

/* ***** FS BLOCKS ***** */

void internal_api_initialize_blocks(){
	char* filePath;
	FILE* bloqueFS;

	for(int i = 0; i < (int)metadataFS->blocks; i++){
		filePath = sindicato_utils_build_block_path(i);

		/* Los archivos se abren con  "a" para que no overrideen el valor que tengan guardados
		 * en caso de reconexion */
		bloqueFS = fopen(filePath, "a");

		fclose(bloqueFS);
		free(filePath);
	}

	log_info(sindicatoDebugLog,"[FILESYSTEM] Bloques inicializados");
}

/* ********** INTERNAL UTILS ********** */

void internal_api_free_array(char** array,int qtyBitsReserved){

	for(int position = 0; position < (qtyBitsReserved - 1); position++){
		free(array[position]);
	}

	free(array);
}

char** internal_api_split_string(char* stringtoSplit, int stringMaxSize, int blocksNeeded){
	char** stringSplitted = malloc(blocksNeeded*sizeof(char)*stringMaxSize);

	int splitFrom = 0;
	int qtyToSplit = stringMaxSize;

	for(int i = 0; i < blocksNeeded; i++){

		stringSplitted[i] = string_substring(stringtoSplit,splitFrom,qtyToSplit);

		splitFrom = splitFrom + qtyToSplit;

		if(splitFrom + qtyToSplit > strlen(stringtoSplit))
			qtyToSplit = strlen(stringtoSplit) - splitFrom;
	}

	return stringSplitted;
}

/* ********** FS FILES ********** */

void internal_api_write_info_file(int stringLenght, char* initialBlock){

}

/* ********** FS BLOCKS ********** */

void internal_api_free_bits_reserved(char** bitsList, int positionThatFailed){

	int block = 0;
	int qtyBitsReserved = positionThatFailed;

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

	for(int blockNumber = 0; blockNumber < metadataFS->blocks; blockNumber++){
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

	for(int i = 0; i < blocksNeeded; i++){
		freeBlock = internal_api_get_free_block();
		if(freeBlock == -1){
			log_error(sindicatoDebugLog, "[FILESYSTEM] ERROR: No hay bloques disponibles");
			internal_api_free_bits_reserved(blocks, i);
			internal_api_free_array(blocks, i);
			return NULL;
		}

		stringAux = string_itoa(freeBlock);
		blocks[i] = string_new();
		string_append(&blocks[i], stringAux);
		free(stringAux);
	}

	log_info(sindicatoDebugLog, "[FILESYSTEM] %d bloques fueron seteados en bitmap.",blocksNeeded);

	return blocks;
}

int internal_api_calculate_blocks_needed(char* fullString){

	int blocksNeeded = 0;
	int stringLenght = string_length(fullString);
	int blockSizeString = (int)(metadataFS->block_size - 4);

	blocksNeeded = stringLenght/blockSizeString;
	if(stringLenght%blockSizeString)
		blocksNeeded++;

	return blocksNeeded;
}

int internal_api_write_block(char* stringToWrite){

	char* blockFullPath;
	int sizeString;
	int stringBlockSize;
	uint32_t pointerNextBlock;

	int qtyblocksNeeded = internal_api_calculate_blocks_needed(stringToWrite);

	char** blocksToWrite = internal_api_get_free_blocks(qtyblocksNeeded);
	if(blocksToWrite == NULL)
		return -1; //error

	//TODO: modo escritura, modo update

	stringBlockSize = (int)metadataFS->block_size - sizeof(uint32_t);
	char** stringToWriteSplitted = internal_api_split_string(stringToWrite, stringBlockSize, qtyblocksNeeded);

	internal_api_write_info_file(strlen(stringToWrite), blocksToWrite[0]);

	for(int i = 0; i < qtyblocksNeeded; i++){
		blockFullPath = sindicato_utils_build_block_path(atoi(blocksToWrite[i]));

		int block = open(blockFullPath, O_RDWR | O_CREAT, 0700);
		ftruncate(block, metadataFS->block_size);

		char* mappedBlock = mmap(0, metadataFS->block_size, PROT_WRITE | PROT_READ, MAP_SHARED, block, 0);

		if(i < (qtyblocksNeeded -1))
			pointerNextBlock = (uint32_t)atoi(blocksToWrite[i+1]);

		sizeString = strlen(stringToWriteSplitted[i]);

		/* Write string spplited in block */
		memcpy(mappedBlock, stringToWriteSplitted[i], sizeString);
		memcpy(mappedBlock + sizeString, &pointerNextBlock , sizeof(uint32_t));

		msync(mappedBlock, metadataFS->block_size, MS_SYNC);

		munmap(mappedBlock, metadataFS->block_size);
		close(block);
	}

	//TODO: IMPORTANTE Return the initial block
	return atoi(blocksToWrite[0]);
}
//restPath, initialBlock, restToSave
void internal_api_create_info_file(char* filePath, int initialBlock, char* stringSaved){

	/* if the file does not exist then create a new one with default keys in blank*/
	if(access(filePath, F_OK) != 0){

		FILE *file = fopen(filePath, "w+");

		fprintf(file, "SIZE=\n");
		fprintf(file, "INITIAL_BLOCK=\n");

		fclose(file);
	}


	char* strInitialBlock = string_itoa(initialBlock);
	char* strSize = string_itoa(strlen(stringSaved));

	/* update the file using config */
	t_config* config = config_create(filePath);

	config_set_value(config, "SIZE", strSize);
	config_set_value(config, "INITIAL_BLOCK", strInitialBlock);

	config_save(config);

	config_destroy(config);

	free(strSize);
	free(strInitialBlock);
}

/* ********************************** PUBLIC  FUNCTIONS ********************************** */

/* Console functions */
void sindicato_api_crear_restaurante(char* nombre, char* cantCocineros, char* posXY, char* afinidadCocinero, char* platos, char* precioPlatos, char* cantHornos){
	log_info(sindicatoLog, "Se creo el restaurante: %s %s %s %s %s %s %s",nombre, cantCocineros, posXY, afinidadCocinero, platos, precioPlatos, cantHornos);

	char* internal_api_build_string_rest(char* cantCocineros, char* posXY, char* afinidadCocinero, char* platos, char* precioPlatos, char* cantHornos){
		char* stringBuilded = string_new();

		string_append(&stringBuilded,"CANTIDAD_COCINEROS=");
		string_append(&stringBuilded,cantCocineros);
		string_append(&stringBuilded,"\n");

		string_append(&stringBuilded,"POSICION=");
		string_append(&stringBuilded,posXY);
		string_append(&stringBuilded,"\n");

		string_append(&stringBuilded,"AFINIDAD_COCINEROS=");
		string_append(&stringBuilded,afinidadCocinero);
		string_append(&stringBuilded,"\n");

		string_append(&stringBuilded,"PLATOS=");
		string_append(&stringBuilded,platos);
		string_append(&stringBuilded,"\n");

		string_append(&stringBuilded,"PRECIO_PLATOS=");
		string_append(&stringBuilded,precioPlatos);
		string_append(&stringBuilded,"\n");

		string_append(&stringBuilded,"CANTIDAD_HORNOS=");
		string_append(&stringBuilded,cantHornos);
		string_append(&stringBuilded,"\n");

		char* cantidadPedidos = "1";
		//TODO: IMPORTANTE Definir como calcular los pedidos en el FS

		string_append(&stringBuilded,"CANTIDAD_PEDIDOS=");
		string_append(&stringBuilded,cantidadPedidos);

		return stringBuilded;
	}

	char* restToSave = internal_api_build_string_rest(cantCocineros, posXY, afinidadCocinero, platos, precioPlatos, cantHornos);

	//TODO: validar que no sea -1
	int initialBlock = internal_api_write_block(restToSave);

	/* Create "info" file */

	char* restPath = sindicato_utils_build_file_full_path(sindicatoRestaurantePath, nombre);

	internal_api_create_info_file(restPath, initialBlock, restToSave);

	free(restPath);
	free(restToSave);
}

void sindicato_api_crear_receta(char* nombre, char* pasos, char* tiempoPasos){
	log_info(sindicatoLog, "Se creo la receta: %s %s %s", nombre, pasos, tiempoPasos);

	char* internal_api_build_string_receta(char* pasos, char* tiempoPasos){
		char* stringBuilded = string_new();

		string_append(&stringBuilded,"PASOS=");
		string_append(&stringBuilded,pasos);
		string_append(&stringBuilded,"\n");

		string_append(&stringBuilded,"TIEMPO_PASOS=");
		string_append(&stringBuilded,tiempoPasos);

		return stringBuilded;
	}

	char* recetaToSave = internal_api_build_string_receta(pasos, tiempoPasos);

	int initialBlock = internal_api_write_block(recetaToSave);


	/* Create "info" file */

	char* restPath = sindicato_utils_build_file_full_path(sindicatoRecetaPath, nombre);

	internal_api_create_info_file(restPath, initialBlock, recetaToSave);

	free(restPath);
	free(recetaToSave);
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

//	f_restaurante* restaurante = read_blocks(0, 188);
//
//	puts("cocineros");
//	puts(string_itoa(restaurante->cantidad_cocineros));
//	puts("hornos");
//	puts(string_itoa(restaurante->cantidad_hornos));
}

void* read_blocks(int initial_block, int size){
	char* path_block;

	int blocks_amount = ceil((float) size / ((float) metadataFS->block_size - 4));

	int next_block = initial_block;

	int offset = 0;

	char* data = malloc(size);

	for(int i = 0; i < blocks_amount; i++){

		path_block = sindicato_utils_build_block_path(next_block);

		int block = open(path_block, O_RDWR | O_CREAT, 0700);
		ftruncate(block, metadataFS->block_size);

		char* mappedBlock = mmap(0, metadataFS->block_size, PROT_WRITE | PROT_READ, MAP_SHARED, block, 0);


		if(i < blocks_amount - 1){
			memcpy(data + offset, mappedBlock, metadataFS->block_size - 4);
			memcpy(&next_block, mappedBlock + metadataFS->block_size - 4,  4);
			offset += metadataFS->block_size - 4;
			size -= metadataFS->block_size - 4;
		}else{
			memcpy(data + offset, mappedBlock, size);
		}

		munmap(mappedBlock, metadataFS->block_size);
		close(block);
	}

	char** array_data = string_split(data, "\n");

	free(data);

	t_config* config_data = array_data_to_config(array_data);

	void* strc_data = config_to_strc(config_data);

	config_destroy(config_data);

	return strc_data;
}

void* config_to_strc(t_config* config_data){

	switch(config_to_file_type(config_data)){
	case F_RESTAURANTE:
		return config_to_restaurante(config_data);
	case F_RECETA:
		return config_to_receta(config_data);
	case F_PEDIDO:
		return config_to_pedido(config_data);
	default:
		return NULL;
	}
}

t_file_type config_to_file_type(t_config* config){
	if(config_has_property(config, D_PASOS)){
		return F_RECETA;
	}else if(config_has_property(config, D_CANTIDAD_COCINEROS)){
		return F_RESTAURANTE;
	}else if(config_has_property(config, D_ESTADO_PEDIDO)){
		return F_PEDIDO;
	}else{
		return -1;
	}
}

f_restaurante* config_to_restaurante(t_config* config_data){
	f_restaurante* restaurante = malloc(sizeof(f_restaurante));

	restaurante->cantidad_hornos = config_get_int_value(config_data, D_CANTIDAD_HORNOS);
	restaurante->cantidad_cocineros = config_get_int_value(config_data, D_CANTIDAD_COCINEROS);
	restaurante->cantidad_pedidos = config_get_int_value(config_data, D_CANTIDAD_PEDIDOS);

	char** aux_array = config_get_array_value(config_data, D_POSICION);

	restaurante->posicion.x = atoi(aux_array[0]);
	restaurante->posicion.y = atoi(aux_array[1]);

	liberar_vector(aux_array);

	restaurante->afinidad_cocineros = list_create();

	aux_array = config_get_array_value(config_data, D_AFINIDAD_COCINEROS);

	int i = 0;

	t_nombre* name;
	while(aux_array[i] != NULL){
		name = malloc(sizeof(t_nombre));
		name->nombre = string_duplicate(aux_array[i]);

		list_add(restaurante->afinidad_cocineros, name);
		i++;
	}

	liberar_vector(aux_array);

	restaurante->platos = list_create();

	aux_array = config_get_array_value(config_data, D_PLATOS);

	i = 0;

	while(aux_array[i] != NULL){
		name = malloc(sizeof(t_nombre));
		name->nombre = string_duplicate(aux_array[i]);

		list_add(restaurante->platos, name);
		i++;
	}

	liberar_vector(aux_array);

	restaurante->precios = list_create();

	aux_array = config_get_array_value(config_data, D_PRECIO_PLATOS);

	i = 0;

	while(aux_array[i] != NULL){
		list_add(restaurante->precios, atoi(aux_array[i]));
		i++;
	}

	liberar_vector(aux_array);

	return restaurante;
}

f_receta* config_to_receta(t_config* config_data){
	f_receta* receta = malloc(sizeof(f_receta));

	receta->pasos = list_create();

	char** aux_pasos = config_get_array_value(config_data, D_PASOS);

	char** aux_tiempos = config_get_array_value(config_data, D_TIEMPO_PASOS);

	int i = 0;

	t_paso* paso;

	while(aux_pasos[i] != NULL){
		paso = malloc(sizeof(t_paso));

		paso->paso.nombre = string_duplicate(aux_pasos[i]);
		paso->duracion = atoi(aux_tiempos[i]);

		list_add(receta->pasos, paso);
		i++;
	}

	liberar_vector(aux_pasos);
	liberar_vector(aux_tiempos);

	return receta;
}

f_pedido* config_to_pedido(t_config* config_data){
	f_pedido* pedido = malloc(sizeof(f_pedido));

	pedido->estado_pedido = string_to_est_pedido(config_get_string_value(config_data, D_ESTADO_PEDIDO));
	pedido->precio_total = config_get_int_value(config_data, D_PRECIO_TOTAL);
	pedido->platos = list_create();

	char** aux_array = config_get_array_value(config_data, D_PLATOS);

	int i = 0;

	t_nombre* name;

	while(aux_array[i] != NULL){
		name = malloc(sizeof(t_nombre));

		name->nombre = string_duplicate(aux_array[i]);

		list_add(pedido->platos, name);
		i++;
	}

	i = 0;

	liberar_vector(aux_array);

	pedido->cantidad_lista = list_create();
	aux_array = config_get_array_value(config_data, D_CANTIDAD_LISTA);

	while(aux_array[i] != NULL){
		list_add(pedido->cantidad_lista, atoi(aux_array[i]));
		i++;
	}

	i = 0;

	liberar_vector(aux_array);

	pedido->cantidad_platos = list_create();
	aux_array = config_get_array_value(config_data, D_CANTIDAD_PLATOS);

	while(aux_array[i] != NULL){
		list_add(pedido->cantidad_platos, atoi(aux_array[i]));
		i++;
	}

	liberar_vector(aux_array);

	return pedido;
}

est_pedido string_to_est_pedido(char* string){
	if(string_equals_ignore_case(string, E_CONFIRMADO)){
		return CONFIRMADO;
	}else if(string_equals_ignore_case(string, E_PENDIENTE)){
		return PENDIENTE;
	}else if(string_equals_ignore_case(string, E_TERMINADO)){
		return TERMINADO;
	}else{
		return -1;
	}
}

t_config* array_data_to_config(char** lineas){
	t_config* config_datos = config_create(sindicato_utils_build_path(sindicatoMountPoint, "/Metadata/Metadata.AFIP"));

	int i = 0;

	while(lineas[i]!=NULL){ //por cada "posicion", hay una linea del vector "lineas" (separado por \n)
		char** key_valor = string_split(lineas[i], "=");
		config_set_value(config_datos, key_valor[0], key_valor[1]); //separo la posicion de la cantidad (a traves del =)y seteo como key la posicion con su valor cantidad
		liberar_vector(key_valor);
		i++;
	}
	return config_datos;
}
