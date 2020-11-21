#include "SindicatoApi.h"

/* ********************************** PRIVATE FUNCTIONS ********************************** */

/* ********** INITIALIZE BASIC COMPONENTS ********** */

/* ***** FS FOLDERS ***** */

void internal_api_createFileSystemFolders(){
	/* Create mount point of the FS */
	sindicato_utils_create_folder(sindicatoMountPoint, true);

	/* Create folder "Metadata" in {mount_point} */
	sindicatoMetadataPath = sindicato_utils_build_path(sindicatoMountPoint, "/Metadata");
	sindicato_utils_create_folder(sindicatoMetadataPath, true);

	/* Create folder "Files" in {mount_point} */
	char* sindicatoFilesPath = sindicato_utils_build_path(sindicatoMountPoint, "/Files");
	sindicato_utils_create_folder(sindicatoFilesPath, true);
	free(sindicatoFilesPath);

	/* Create folder "Blocks" in {mount_point} */
	sindicatoBlocksPath = sindicato_utils_build_path(sindicatoMountPoint, "/Blocks");
	sindicato_utils_create_folder(sindicatoBlocksPath, true);


	/* Create folder "Receta" in {mount_point}/Files */
	sindicatoRecetaPath = sindicato_utils_build_path(sindicatoMountPoint, "/Files/Recetas/");
	sindicato_utils_create_folder(sindicatoRecetaPath, true);

	/* Create folder "Restaurante" in {mount_point}/Files */
	sindicatoRestaurantePath = sindicato_utils_build_path(sindicatoMountPoint, "/Files/Restaurantes/");
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
	// TODO: validar multiplo de 8

	int bitarrayFile = open(bitmapPathFS, O_RDWR | O_CREAT, 0700);

	ftruncate(bitarrayFile, blocks);

	char* bitarrayMap = mmap(0, blocks, PROT_WRITE | PROT_READ, MAP_SHARED, bitarrayFile, 0);

	// TODO: ver errores en mapeo

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

t_config* internal_api_array_to_config(char** stringArray){
	char* metadataFile = sindicato_utils_build_path(sindicatoMetadataPath, "/Metadata.AFIP");
	t_config* config = config_create(metadataFile);

	int i = 0;

	while(stringArray[i] != NULL){ //por cada "posicion", hay una linea del vector "stringArray" (separado por \n)
		char** key_valor = string_split(stringArray[i], "=");
		config_set_value(config, key_valor[0], key_valor[1]); //separo la posicion de la cantidad (a traves del =)y seteo como key la posicion con su valor cantidad
		liberar_vector(key_valor);
		i++;
	}

	free(metadataFile);

	return config;
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

est_pedido internal_api_string_to_estado_pedido(char* string){
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

file_type internal_api_get_file_type(t_config* config){
	if(config_has_property(config, D_PASOS)){
		return TYPE_RECETA;
	}else if(config_has_property(config, D_CANTIDAD_COCINEROS)){
		return TYPE_RESTAURANTE;
	}else if(config_has_property(config, D_ESTADO_PEDIDO)){
		return TYPE_PEDIDO;
	}else{
		return -1;
	}
}

t_restaurante_file* internal_api_config_to_restaurante(t_config* dataConfig){
	t_restaurante_file* restaurante = malloc(sizeof(t_restaurante_file));

	restaurante->cantidad_hornos = config_get_int_value(dataConfig, D_CANTIDAD_HORNOS);
	restaurante->cantidad_cocineros = config_get_int_value(dataConfig, D_CANTIDAD_COCINEROS);
	restaurante->cantidad_pedidos = config_get_int_value(dataConfig, D_CANTIDAD_PEDIDOS);

	char** aux_array = config_get_array_value(dataConfig, D_POSICION);

	restaurante->posicion.x = atoi(aux_array[0]);
	restaurante->posicion.y = atoi(aux_array[1]);

	liberar_vector(aux_array);

	restaurante->afinidad_cocineros = list_create();

	aux_array = config_get_array_value(dataConfig, D_AFINIDAD_COCINEROS);

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

	aux_array = config_get_array_value(dataConfig, D_PLATOS);

	i = 0;

	while(aux_array[i] != NULL){
		name = malloc(sizeof(t_nombre));
		name->nombre = string_duplicate(aux_array[i]);

		list_add(restaurante->platos, name);
		i++;
	}

	liberar_vector(aux_array);

	restaurante->precios = list_create();

	aux_array = config_get_array_value(dataConfig, D_PRECIO_PLATOS);

	i = 0;

	while(aux_array[i] != NULL){
		list_add(restaurante->precios, (void *) atoi(aux_array[i]));
		i++;
	}

	liberar_vector(aux_array);

	return restaurante;
}

t_receta_file* internal_api_config_to_receta(t_config* dataConfig){
	t_receta_file* receta = malloc(sizeof(t_receta_file));

	receta->pasos = list_create();

	char** aux_pasos = config_get_array_value(dataConfig, D_PASOS);

	char** aux_tiempos = config_get_array_value(dataConfig, D_TIEMPO_PASOS);

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

t_pedido_file* internal_api_config_to_pedido(t_config* dataConfig){
	t_pedido_file* pedido = malloc(sizeof(t_pedido_file));

	pedido->estado_pedido = internal_api_string_to_estado_pedido(config_get_string_value(dataConfig, D_ESTADO_PEDIDO));
	pedido->precio_total = config_get_int_value(dataConfig, D_PRECIO_TOTAL);
	pedido->platos = list_create();

	char** aux_array = config_get_array_value(dataConfig, D_PLATOS);

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
	aux_array = config_get_array_value(dataConfig, D_CANTIDAD_LISTA);

	while(aux_array[i] != NULL){
		list_add(pedido->cantidad_lista, (void*) atoi(aux_array[i]));
		i++;
	}

	i = 0;

	liberar_vector(aux_array);

	pedido->cantidad_platos = list_create();
	aux_array = config_get_array_value(dataConfig, D_CANTIDAD_PLATOS);

	while(aux_array[i] != NULL){
		list_add(pedido->cantidad_platos, (void*) atoi(aux_array[i]));
		i++;
	}

	liberar_vector(aux_array);

	return pedido;
}

void* internal_api_config_to_structure(t_config* dataConfig){

	switch(internal_api_get_file_type(dataConfig)){
	case TYPE_RESTAURANTE:
		return internal_api_config_to_restaurante(dataConfig);
	case TYPE_RECETA:
		return internal_api_config_to_receta(dataConfig);
	case TYPE_PEDIDO:
		return internal_api_config_to_pedido(dataConfig);
	default:
		return NULL;
	}
}

void internal_api_free_blocks_array(char** array,int qtyBitsReserved){

	for(int position = 0; position < (qtyBitsReserved - 1); position++){
		free(array[position]);
	}

	free(array);
}

void internal_api_free_array(char** array){

		int position = 0;

		while(array[position] != NULL){
			free(array[position]);
			position++;
		}

		free(array);
}

/* ********** FS FILES ********** */

t_initialBlockInfo* internal_api_read_initial_block_info(char* path){
	t_initialBlockInfo* initialBlock = malloc(sizeof(t_initialBlockInfo));

	t_config* blockInfo = config_create(path);

	initialBlock->initialBlock = (uint32_t) config_get_int_value(blockInfo, "INITIAL_BLOCK");
	initialBlock->stringSize = config_get_int_value(blockInfo,"SIZE");

	config_destroy(blockInfo);

	return initialBlock;
}

/**
 * Parametros
 * 	name: nombre del archivo, sin extension
 * 	restaurateOfPedido: nombre del restaurante correspondiente al name del pedido, sin extension
 * 	fileType: tipo de archivo,
 * 		TYPE_RESTAURANTE
 * 		TYPE_RECETA
 * 		TYPE_PEDIDO
 *
 * Retorno
 * 	t_initialBlockInfo initialBlock, corresponde a una estructura con el bloque inicial y largo del bloque
 * 	Se debe liberar la estructura una vez finalizado su uso.
 *
 * Error
 * 	Retorna NULL
 */

t_initialBlockInfo* internal_api_get_initial_block_info(char* name, char* restaurateOfPedido, file_type fileType){
	t_initialBlockInfo* initialBlock = malloc(sizeof(t_initialBlockInfo));

	char* filePath;

	switch(fileType){
	case(TYPE_RESTAURANTE):
		filePath = sindicato_utils_build_file_full_path(sindicatoRestaurantePath, name, true, NULL);
		break;
	case(TYPE_RECETA):
		filePath = sindicato_utils_build_file_full_path(sindicatoRecetaPath, name, false, NULL);
		break;
	case(TYPE_PEDIDO):
		filePath = sindicato_utils_build_file_full_path(sindicatoRecetaPath, name, false, restaurateOfPedido);
		break;
	default:
		return NULL;
		break;
	}

	initialBlock = internal_api_read_initial_block_info(filePath);

	free(filePath);

	return initialBlock;
}

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
			internal_api_free_blocks_array(blocks, i);
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

char* internal_api_get_string_from_filesystem(int initialBlock, int stringSize){
	char* blockPath;

	int offset = 0;

	int blocksQty = ceil((float) stringSize / ((float) (metadataFS->block_size - 4)));
	int nextBlock = initialBlock;

	char* blockData = malloc(stringSize*sizeof(char)+1);
	memset(blockData, 0, stringSize*sizeof(char)+1);

	for(int i = 0; i < blocksQty; i++){

		blockPath = sindicato_utils_build_block_path(nextBlock);

		int block = open(blockPath, O_RDWR | O_CREAT, 0700);
		ftruncate(block, metadataFS->block_size);

		char* blockMapped = mmap(0, metadataFS->block_size, PROT_WRITE | PROT_READ, MAP_SHARED, block, 0);

		if(i < blocksQty - 1){
			memcpy(blockData + offset, blockMapped, metadataFS->block_size - 4);
			memcpy(&nextBlock, blockMapped + metadataFS->block_size - 4,  4);
			offset += metadataFS->block_size - 4;
			stringSize -= metadataFS->block_size - 4;
		}else{
			memcpy(blockData + offset, blockMapped, stringSize);
		}

		munmap(blockMapped, metadataFS->block_size);
		close(block);
		free(blockPath);
	}

	return blockData;
}


int internal_api_write_block(char* stringToWrite, t_initialBlockInfo* initialBLock, mode_fs mode){

	char* blockFullPath;
	int sizeString;
	int stringBlockSize;
	uint32_t pointerNextBlock;

	char** blocksToWrite;

	int qtyblocksNeeded = internal_api_calculate_blocks_needed(stringToWrite);

	if(mode == MODE_ADD){

		blocksToWrite = internal_api_get_free_blocks(qtyblocksNeeded);
		if(blocksToWrite == NULL)
			return -1; //error
	}

	if(mode == MODE_UPDATE){

		char* originalStringStored = internal_api_get_string_from_filesystem(initialBLock->initialBlock,initialBLock->stringSize);
		int qtyBlocksStored = internal_api_calculate_blocks_needed(originalStringStored);

		int blocksMissed = 0;

		if(qtyblocksNeeded > qtyBlocksStored){
			blocksMissed = qtyblocksNeeded - qtyBlocksStored;
			//TODO: Obtener el o los bloques faltantes
		} else if (qtyblocksNeeded < qtyBlocksStored){
			blocksMissed = qtyBlocksStored - qtyblocksNeeded;
			//TODO: liberar el ultimo bloque

		}
		//TODO: Armar con los bloques ya existentes y validar si necesito alguno mas

	}

	stringBlockSize = (int)metadataFS->block_size - sizeof(uint32_t);
	char** stringToWriteSplitted = internal_api_split_string(stringToWrite, stringBlockSize, qtyblocksNeeded);

	for(int i = 0; i < qtyblocksNeeded; i++){
		blockFullPath = sindicato_utils_build_block_path(atoi(blocksToWrite[i]));

		int block = open(blockFullPath, O_RDWR | O_CREAT, 0700);
		ftruncate(block, metadataFS->block_size);

		char* blockMapped = mmap(0, metadataFS->block_size, PROT_WRITE | PROT_READ, MAP_SHARED, block, 0);

		if(i < (qtyblocksNeeded -1))
			pointerNextBlock = (uint32_t)atoi(blocksToWrite[i+1]);

		sizeString = strlen(stringToWriteSplitted[i]);

		/* Write string spplited in block */
		memcpy(blockMapped, stringToWriteSplitted[i], sizeString);
		memcpy(blockMapped + sizeString, &pointerNextBlock , sizeof(uint32_t));

		msync(blockMapped, metadataFS->block_size, MS_SYNC);

		munmap(blockMapped, metadataFS->block_size);
		close(block);
	}

	return atoi(blocksToWrite[0]);
}

/* If fails return NULL */
void* internal_api_read_blocks(int initialBlock, int stringSize){
	char* blockPath;

	int offset = 0;

	int blocksQty = ceil((float) stringSize / ((float) (metadataFS->block_size - 4)));
	int nextBlock = initialBlock;

	char* blockData = malloc(stringSize*sizeof(char)+1);
	memset(blockData, 0, stringSize*sizeof(char)+1);

	for(int i = 0; i < blocksQty; i++){

		blockPath = sindicato_utils_build_block_path(nextBlock);

		int block = open(blockPath, O_RDWR | O_CREAT, 0700);
		ftruncate(block, metadataFS->block_size);

		char* blockMapped = mmap(0, metadataFS->block_size, PROT_WRITE | PROT_READ, MAP_SHARED, block, 0);

		if(i < blocksQty - 1){
			memcpy(blockData + offset, blockMapped, metadataFS->block_size - 4);
			memcpy(&nextBlock, blockMapped + metadataFS->block_size - 4,  4);
			offset += metadataFS->block_size - 4;
			stringSize -= metadataFS->block_size - 4;
		}else{
			memcpy(blockData + offset, blockMapped, stringSize);
		}

		munmap(blockMapped, metadataFS->block_size);
		close(block);
		free(blockPath);
	}

	char** blockDataArray = string_split(blockData, "\n");

	t_config* dataConfig = internal_api_array_to_config(blockDataArray);

	void* strc_data = internal_api_config_to_structure(dataConfig);

	config_destroy(dataConfig);
	liberar_vector(blockDataArray);
	free(blockData);

	return strc_data;
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
		// TODO: IMPORTANTE Definir como calcular los pedidos en el FS

		string_append(&stringBuilded,"CANTIDAD_PEDIDOS=");
		string_append(&stringBuilded,cantidadPedidos);

		return stringBuilded;
	}

	char* restToSave = internal_api_build_string_rest(cantCocineros, posXY, afinidadCocinero, platos, precioPlatos, cantHornos);

	// TODO: validar que no sea -1
	int initialBlock = internal_api_write_block(restToSave, NULL, MODE_ADD);


	/* Create restaurante folder */
	char* restPath = sindicato_utils_build_path(sindicatoRestaurantePath, nombre);
	sindicato_utils_create_folder(restPath, true);

	/* Create "info" file */
	restPath = sindicato_utils_build_file_full_path(sindicatoRestaurantePath, nombre, true, NULL);
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

	int initialBlock = internal_api_write_block(recetaToSave, NULL, MODE_ADD);


	/* Create "info" file */

	char* restPath = sindicato_utils_build_file_full_path(sindicatoRecetaPath, nombre, false, NULL);

	internal_api_create_info_file(restPath, initialBlock, recetaToSave);

	free(restPath);
	free(recetaToSave);
}

/* Server functions */
void sindicato_api_send_response_of_operation(t_responseMessage* response){

	if(response->message->parametros == NULL)
		response->message->tipo_mensaje = ERROR;

	loggear_mensaje_enviado(response->message->parametros, response->message->tipo_mensaje, sindicatoLog);
	enviar_mensaje(response->message, response->socket);
}

t_restaurante_y_plato* sindicato_api_consultar_platos(void* consultaPatos){
	t_nombre* restaurante = consultaPatos;

	char* restauranteFilePath = sindicato_utils_build_file_full_path(sindicatoRestaurantePath, restaurante->nombre, true, NULL);
	if(sindicato_utils_verify_if_file_exist(restauranteFilePath)){
		log_info(sindicatoDebugLog, "%s existe", restauranteFilePath);
	} else {
		log_error(sindicatoDebugLog,  "%s NO existe", restauranteFilePath);
		free(restauranteFilePath);
		free(restaurante);
		return NULL;
	}

	/* Initialize of platos structure */
	t_restaurante_y_plato* platos = malloc(sizeof(t_restaurante_y_plato));
	platos->nombres = list_create();

	t_initialBlockInfo* initialBLock = internal_api_get_initial_block_info(restaurante->nombre,NULL,TYPE_RESTAURANTE);

	t_restaurante_file* restauranteInfo = internal_api_read_blocks(initialBLock->initialBlock, initialBLock->stringSize);

	platos->nombres = restauranteInfo->platos;
	platos->cantElementos = restauranteInfo->platos->elements_count;

	free(restauranteInfo);
	free(initialBLock);

	return platos;
}

uint32_t* sindicato_api_guardar_pedido(void* pedido){
	// t_nombre_y_id* asd;

	uint32_t* opResult = malloc(sizeof(uint32_t));
	/* DELETE THIS: datos dummies solo para TEST */
	(*opResult) = 1;

	return opResult;
}

uint32_t* sindicato_api_guardar_plato(void* pedido){
	// m_guardarPlato* asd;

	uint32_t* opResult = malloc(sizeof(uint32_t));
	/* DELETE THIS: datos dummies solo para TEST */
	(*opResult) = 1;

	return opResult;
}

uint32_t* sindicato_api_confirmar_pedido(void* pedido){
	// t_nombre_y_id* asd;
	uint32_t* opResult = malloc(sizeof(uint32_t));
	/* DELETE THIS: datos dummies solo para TEST */
	(*opResult) = 1;

	return opResult;
}

rta_obtenerPedido* sindicato_api_obtener_pedido(void* Consultapedido){
	// t_nombre_y_id* asd;
	rta_obtenerPedido* pedido = malloc(sizeof(rta_obtenerPedido));
	pedido->infoPedidos = list_create();

	t_elemPedido* pedidoElem = malloc(sizeof(t_elemPedido));

	/* DELETE THIS: datos dummies solo para TEST */
	pedidoElem->cantHecha = 1;
	pedidoElem->cantTotal = 1;
	pedidoElem->comida.nombre = string_duplicate("Milanesa");

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
	recetaPrecio->receta.nombre = string_duplicate("Milanesa");
	recetaPrecio->precio = 500;

	afinidad->nombre = string_duplicate("Empanadas");

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
	// m_platoListo* asd;
	uint32_t* opResult = malloc(sizeof(uint32_t));
	/* DELETE THIS: datos dummies solo para TEST */
	(*opResult) = 1;

	return opResult;
}

rta_obtenerReceta* sindicato_api_obtener_receta(void* plato){
	// t_nombre* asd;
	rta_obtenerReceta* receta = malloc(sizeof(rta_obtenerReceta));
	receta->pasos = list_create();

	t_paso* paso = malloc(sizeof(t_paso));
	paso->duracion = 1;
	paso->paso.nombre = string_duplicate("Milanesear");
	puts(paso->paso.nombre);

	/* DELETE THIS: datos dummies solo para TEST */
	list_add(receta->pasos,paso);
	receta->cantPasos = 1;

	return receta;
}

uint32_t* sindicato_api_terminar_pedido(void* pedido){
	// t_nombre_y_id * asd;
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

	/* DELETE THIS */
	/*t_restaurante_file* restaurante = internal_api_read_blocks(0, 188);

	puts("cocineros");
	char* cocineros = string_itoa(restaurante->cantidad_cocineros);
	puts(cocineros);
	puts("hornos");
	char* hornos = string_itoa(restaurante->cantidad_hornos);
	puts(hornos);

	char* fullString = internal_api_get_string_from_filesystem(0, 188);
	puts(fullString);
	free(fullString);

	t_initialBlockInfo* blockInit = internal_api_read_initial_block_info("/home/utnso/workspace/tp-2020-2c-CoronaLinux/Sindicato/afip/Files/Restaurantes/LaParri/LaParri.AFIP");
	puts(string_itoa((int)blockInit->initialBlock));
	puts(string_itoa(blockInit->stringSize));
	free(blockInit);*/

	/*puts(sindicato_utils_build_file_full_path(sindicatoRestaurantePath, "Laparri3", true, NULL));
	puts(sindicato_utils_build_file_full_path(sindicatoRestaurantePath, "Pedido1", false, "Laparri"));
	puts(sindicato_utils_build_file_full_path(sindicatoRecetaPath, "Laparri3", false, NULL));*/


	/*free(cocineros);
	free(hornos);
	free(restaurante);*/
}
