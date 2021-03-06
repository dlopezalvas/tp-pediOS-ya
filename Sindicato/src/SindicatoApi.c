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

	log_info(sindicatoDebugLog,"[FILESYSTEM] - Metadata leida y guardada");

	free(metadaPathFS);
}

/* ***** FS BITMAP ***** */

void internal_api_bitmap_create(){
	char* bitmapPathFS = sindicato_utils_build_path(sindicatoMountPoint, "/Metadata/Bitmap.bin");

	int blocks = metadataFS->blocks/8;

	int bitarrayFile = open(bitmapPathFS, O_RDWR | O_CREAT, 0700);

	ftruncate(bitarrayFile, blocks);

	char* bitarrayMap = mmap(0, blocks, PROT_WRITE | PROT_READ, MAP_SHARED, bitarrayFile, 0);

	bitarray = bitarray_create_with_mode(bitarrayMap, blocks, LSB_FIRST);

	msync(bitarray, sizeof(bitarray), MS_SYNC);

	pthread_mutex_init(&bitarray_mtx, NULL);

	log_info(sindicatoDebugLog,"[BITMAP] - Bitmap creado");

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

	log_info(sindicatoDebugLog,"[BLOCKS] - Bloques inicializados");
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

char* internal_api_list_to_string(t_list* list, int type){
	char* listString = string_new();

	string_append(&listString, "[");
	for(int i = 0; i < list->elements_count; i++){

		if(type == 1){
			uint32_t element = (uint32_t)list_get(list, i);
			if(i == 0){

				string_append_with_format(&listString, "%d", element);
			}else
				string_append_with_format(&listString, ",%d", element);

		}else{
			t_nombre* name = list_get(list, i);
			if(i == 0)
				string_append_with_format(&listString, "%s", name->nombre);
			else
				string_append_with_format(&listString, ",%s", name->nombre);
		}
	}
	string_append(&listString, "]");

	return listString;
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

	char** aux_array = config_get_array_value(dataConfig, D_LISTA_PLATOS);

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

void free_nombre(t_nombre* mensaje){
	free(mensaje->nombre);
	free(mensaje);
}

void free_number(uint32_t* mensaje){
	free(mensaje);
}

void internal_api_free_restaurante_file(t_restaurante_file* restaurante, op_code type){

	switch(type){
		case(CONSULTAR_PLATOS):
			list_destroy_and_destroy_elements(restaurante->afinidad_cocineros, (void*) free_nombre);
			list_destroy(restaurante->platos);
			list_destroy(restaurante->precios);
			break;
		case(OBTENER_RESTAURANTE):
			list_destroy(restaurante->afinidad_cocineros);
			list_destroy_and_destroy_elements(restaurante->platos, (void*) free_nombre);
			list_destroy(restaurante->precios);
			break;
		case(GUARDAR_PLATO):
			list_destroy_and_destroy_elements(restaurante->afinidad_cocineros, (void*) free_nombre);
			list_destroy_and_destroy_elements(restaurante->platos, (void*) free_nombre);
			list_destroy(restaurante->precios);
			break;
		default:
			break;
	}

	free(restaurante);
}

void internal_api_free_pedido(t_pedido_file* pedido, op_code code){

	switch(code){
		case(GUARDAR_PLATO):
			list_destroy(pedido->platos);
			list_destroy(pedido->cantidad_platos);
			list_destroy(pedido->cantidad_lista);
			break;
		case(CONFIRMAR_PEDIDO):
			list_destroy_and_destroy_elements(pedido->platos, (void*) free_nombre);
			list_destroy(pedido->cantidad_platos);
			list_destroy(pedido->cantidad_lista);
			break;
		case(PLATO_LISTO):
			list_destroy_and_destroy_elements(pedido->platos, (void*) free_nombre);
			list_destroy(pedido->cantidad_platos);
			list_destroy(pedido->cantidad_lista);
			break;
		case(TERMINAR_PEDIDO):
			list_destroy_and_destroy_elements(pedido->platos, (void*) free_nombre);
			list_destroy(pedido->cantidad_platos);
			list_destroy(pedido->cantidad_lista);
			break;
		case(OBTENER_PEDIDO):
			list_destroy_and_destroy_elements(pedido->platos, (void*) free_nombre);
			list_destroy(pedido->cantidad_platos);
			list_destroy(pedido->cantidad_lista);
			break;
		default:
			break;
	}

	free(pedido);
}

char* internal_api_restaurante_to_string(char* cantCocineros, char* posXY, char* afinidadCocinero, char* platos, char* precioPlatos, char* cantHornos, char* cantPedidos){
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

	char* cantidadPedidos = cantPedidos;

	string_append(&stringBuilded,"CANTIDAD_PEDIDOS=");
	string_append(&stringBuilded,cantidadPedidos);

	return stringBuilded;
}

char* internal_api_receta_to_string(char* pasos, char* tiempoPasos){
	char* stringBuilded = string_new();

	string_append(&stringBuilded,"PASOS=");
	string_append(&stringBuilded,pasos);
	string_append(&stringBuilded,"\n");

	string_append(&stringBuilded,"TIEMPO_PASOS=");
	string_append(&stringBuilded,tiempoPasos);

	return stringBuilded;
}

char* internal_api_pedido_to_string(char* estado, char* platos, char* cantPlatos,char* cantLista, char* precioTotal){
	char* stringBuilded = string_new();

	string_append(&stringBuilded, "ESTADO_PEDIDO=");
	string_append(&stringBuilded, estado);
	string_append(&stringBuilded, "\n");

	string_append(&stringBuilded, "LISTA_PLATOS=");
	string_append(&stringBuilded, platos);
	string_append(&stringBuilded, "\n");

	string_append(&stringBuilded, "CANTIDAD_PLATOS=");
	string_append(&stringBuilded, cantPlatos);
	string_append(&stringBuilded, "\n");

	string_append(&stringBuilded, "CANTIDAD_LISTA=");
	string_append(&stringBuilded, cantLista);
	string_append(&stringBuilded, "\n");

	string_append(&stringBuilded, "PRECIO_TOTAL=");
	string_append(&stringBuilded, precioTotal);

	return stringBuilded;
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

	char* filePath;

	switch(fileType){
	case(TYPE_RESTAURANTE):
		filePath = sindicato_utils_build_file_full_path(sindicatoRestaurantePath, name, true, NULL);
		break;
	case(TYPE_RECETA):
		filePath = sindicato_utils_build_file_full_path(sindicatoRecetaPath, name, false, NULL);
		break;
	case(TYPE_PEDIDO):
		filePath = sindicato_utils_build_file_full_path(sindicatoRestaurantePath, name, false, restaurateOfPedido);
		break;
	default:
		return NULL;
		break;
	}

	t_initialBlockInfo* initialBlock = internal_api_read_initial_block_info(filePath);

	free(filePath);

	return initialBlock;
}

void internal_api_write_info_file(char* filePath, int initialBlock, char* stringSaved){

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

void internal_api_free_bit(int block){
	if(bitarray_test_bit(bitarray, block)){

		/* BEGIN CRITICAL SECTION bitarray_mtx */
		pthread_mutex_lock(&bitarray_mtx);

		bitarray_clean_bit(bitarray, block);
		msync(bitarray, sizeof(bitarray), MS_SYNC);

		pthread_mutex_unlock(&bitarray_mtx);
		/* END   CRITICAL SECTION bitarray_mtx */

		log_info(sindicatoLog,"[BITMAP] - Bloque %d liberado", block);
	}
}

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

			log_info(sindicatoLog,"[BITMAP] - Bloque %d liberado", block);
		}
	}

	log_info(sindicatoDebugLog, "[BITMAP] - %d bloques reservados fueron liberados en bitmap.",qtyBitsReserved);
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

			log_info(sindicatoLog,"[BITMAP] - Bloque %d reservado", blockNumber);

			return blockNumber;
		}
	}

	return -1;
}

char** internal_api_get_free_blocks(int blocksNeeded){

	char* stringAux;

	int freeBlock = 0;
	char** blocks = malloc(blocksNeeded*sizeof(char*));

	for(int i = 0; i < blocksNeeded; i++){
		freeBlock = internal_api_get_free_block();
		if(freeBlock == -1){
			log_error(sindicatoDebugLog, "[BITMAP] - ERROR: No hay bloques disponibles");
			log_error(sindicatoLog, "[BITMAP] - ERROR: No hay bloques disponibles");
			internal_api_free_bits_reserved(blocks, i);
			internal_api_free_blocks_array(blocks, i);
			return NULL;
		}

		stringAux = string_itoa(freeBlock);
		blocks[i] = string_new();
		string_append(&blocks[i], stringAux);
		free(stringAux);
	}

	log_info(sindicatoLog, "[BITMAP] - %d bloques fueron seteados en bitmap.",blocksNeeded);
	log_info(sindicatoDebugLog, "[BITMAP] - %d bloques fueron seteados en bitmap.",blocksNeeded);

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

char* internal_api_get_string_from_filesystem(int initialBlock, int stringSize, char** blockList){
	char* blockPath;
	*blockList = string_new();

	int offset = 0;

	int blocksQty = ceil((float) stringSize / ((float) (metadataFS->block_size - 4)));
	int nextBlock = initialBlock;

	char* blockData = malloc(stringSize*sizeof(char)+1);
	memset(blockData, 0, stringSize*sizeof(char)+1);
	string_append_with_format(blockList,"%d", nextBlock );
	for(int i = 0; i < blocksQty; i++){

		blockPath = sindicato_utils_build_block_path(nextBlock);

		int block = open(blockPath, O_RDWR | O_CREAT, 0700);
		ftruncate(block, metadataFS->block_size);

		char* blockMapped = mmap(0, metadataFS->block_size, PROT_WRITE | PROT_READ, MAP_SHARED, block, 0);

		if(i < blocksQty - 1){
			memcpy(blockData + offset, blockMapped, metadataFS->block_size - 4);
			memcpy(&nextBlock, blockMapped + metadataFS->block_size - 4,  4);
			string_append_with_format(blockList,",%d", nextBlock );
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
		if(blocksToWrite == NULL){
			return ERROR_WRITE_BLOCK;
		}
	}

	if(mode == MODE_UPDATE){

		char* originalBlocks;

		char* originalStringStored = internal_api_get_string_from_filesystem(initialBLock->initialBlock,initialBLock->stringSize, &originalBlocks);
		int qtyBlocksStored = internal_api_calculate_blocks_needed(originalStringStored);
		free(originalStringStored);

		int blocksMissed = 0;

		if(qtyblocksNeeded > qtyBlocksStored){

			blocksMissed = qtyblocksNeeded - qtyBlocksStored;

			char** newBlocks = internal_api_get_free_blocks(blocksMissed);

			for(int i = 0; i < blocksMissed; i++){
				string_append_with_format(&originalBlocks, ",%s", newBlocks[i]);
			}

			blocksToWrite = string_split(originalBlocks, ",");

			free(newBlocks);

		} else if (qtyblocksNeeded < qtyBlocksStored){

			blocksToWrite = string_split(originalBlocks, ",");

			char* block = blocksToWrite[qtyBlocksStored-1];
			int blockToDelete = atoi(block);

			internal_api_free_bit(blockToDelete);
		} else {
			blocksToWrite = string_split(originalBlocks, ",");
		}
		free(originalBlocks);
	}

	int initBlock;
	stringBlockSize = (int)metadataFS->block_size - sizeof(uint32_t);
	char** stringToWriteSplitted = internal_api_split_string(stringToWrite, stringBlockSize, qtyblocksNeeded);

	for(int i = 0; i < qtyblocksNeeded; i++){
		blockFullPath = sindicato_utils_build_block_path(atoi(blocksToWrite[i]));

		int block = open(blockFullPath, O_RDWR | O_CREAT, 0700);
		ftruncate(block, metadataFS->block_size);

		char* blockMapped = mmap(0, metadataFS->block_size, PROT_WRITE | PROT_READ, MAP_SHARED, block, 0);

		if(i == 0)
			initBlock = atoi(blocksToWrite[0]);

		if(i < (qtyblocksNeeded -1))
			pointerNextBlock = (uint32_t)atoi(blocksToWrite[i+1]);

		sizeString = strlen(stringToWriteSplitted[i]);

		/* Write string spplited in block */
		memcpy(blockMapped, stringToWriteSplitted[i], sizeString);
		memcpy(blockMapped + sizeString, &pointerNextBlock , sizeof(uint32_t));

		msync(blockMapped, metadataFS->block_size, MS_SYNC);

		free(stringToWriteSplitted[i]);
		free(blocksToWrite[i]);

		munmap(blockMapped, metadataFS->block_size);
		close(block);
		free(blockFullPath);
	}

	free(stringToWriteSplitted);
	free(blocksToWrite);

	return initBlock;
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

	if(!sindicato_utils_verify_if_file_integrity_is_ok(nombre, TYPE_RESTAURANTE_INICIAL, 0, NULL)){
		log_error(sindicatoLog, "[FILESYSTEM] - No se pudo crear restaurante: %s %s %s %s %s %s %s",
				nombre, cantCocineros, posXY, afinidadCocinero, platos, precioPlatos, cantHornos);
		log_error(sindicatoLog, "[FILESYSTEM] - El restaurante ya existe en el FS");

		return;
	}

	char* restToSave = internal_api_restaurante_to_string(cantCocineros, posXY, afinidadCocinero, platos, precioPlatos, cantHornos, "0");

	/* Create restaurante folder */
	char* restPath = sindicato_utils_build_path(sindicatoRestaurantePath, nombre);
	sindicato_utils_create_folder(restPath, true);

	int initialBlock = internal_api_write_block(restToSave, NULL, MODE_ADD);
	if(initialBlock == ERROR_WRITE_BLOCK){

		log_error(sindicatoLog, "[FILESYSTEM] - No se pudo crear restaurante: %s %s %s %s %s %s %s",
				nombre, cantCocineros, posXY, afinidadCocinero, platos, precioPlatos, cantHornos);
		log_error(sindicatoLog, "[FILESYSTEM] - Fallo la escritura del bloque, validar parametros del comando");

		free(restPath);
		free(restToSave);

		return;
	}

	free(restPath);

	/* Create "info" file */
	restPath = sindicato_utils_build_file_full_path(sindicatoRestaurantePath, nombre, true, NULL);

	pthread_mutex_t* restauranteMutex = internal_utils_get_mutex(nombre);

	pthread_mutex_lock(restauranteMutex);
	internal_api_write_info_file(restPath, initialBlock, restToSave);
	pthread_mutex_unlock(restauranteMutex);
	log_info(sindicatoLog, "[FILESYSTEM] - Se creo restaurante: %s %s %s %s %s %s %s",
			nombre, cantCocineros, posXY, afinidadCocinero, platos, precioPlatos, cantHornos);

	free(restPath);
	free(restToSave);
}

void sindicato_api_crear_receta(char* nombre, char* pasos, char* tiempoPasos){

	if(!sindicato_utils_verify_if_file_integrity_is_ok(nombre, TYPE_RECETA_INICIAL, 0, NULL)){
		log_error(sindicatoLog, "[FILESYSTEM] - No se pudo crear la receta: %s %s %s", nombre, pasos, tiempoPasos);
		log_error(sindicatoLog, "[FILESYSTEM] - La Receta ya existe en el FS");

		return;
	}

	char* recetaToSave = internal_api_receta_to_string(pasos, tiempoPasos);

	int initialBlock = internal_api_write_block(recetaToSave, NULL, MODE_ADD);
	if(initialBlock == ERROR_WRITE_BLOCK){

		log_error(sindicatoLog, "[FILESYSTEM] - No se pudo crear la receta: %s %s %s", nombre, pasos, tiempoPasos);
		log_error(sindicatoLog, "[FILESYSTEM] - Fallo la escritura del bloque, validar parametros del comando");

		free(recetaToSave);

		return;
	}

	/* Create "info" file */
	char* restPath = sindicato_utils_build_file_full_path(sindicatoRecetaPath, nombre, false, NULL);

	pthread_mutex_t* recetaMutex = internal_utils_get_mutex(nombre);

	pthread_mutex_lock(recetaMutex);
	internal_api_write_info_file(restPath, initialBlock, recetaToSave);
	pthread_mutex_unlock(recetaMutex);

	log_info(sindicatoLog, "[FILESYSTEM] - Se creo la receta: %s %s %s", nombre, pasos, tiempoPasos);

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

	if(!sindicato_utils_verify_if_exist(restaurante->nombre, NULL, TYPE_RESTAURANTE)){

		log_error(sindicatoLog, "[FILESYSTEM] - El restaurante no existe");
		free(restaurante->nombre);
		free(restaurante);
		return NULL;
	}

	/* Initialize of platos structure */
	t_restaurante_y_plato* platos = malloc(sizeof(t_restaurante_y_plato));

	pthread_mutex_t* restauranteMutex = internal_utils_get_mutex(restaurante->nombre);
	pthread_mutex_lock(restauranteMutex);
	/* get the info from FS */
	t_initialBlockInfo* initialBLock = internal_api_get_initial_block_info(restaurante->nombre,NULL,TYPE_RESTAURANTE);
	pthread_mutex_unlock(restauranteMutex);
	t_restaurante_file* restauranteInfo = internal_api_read_blocks(initialBLock->initialBlock, initialBLock->stringSize);

	platos->nombres = list_duplicate(restauranteInfo->platos);
	platos->cantElementos = platos->nombres->elements_count;

	internal_api_free_restaurante_file(restauranteInfo, CONSULTAR_PLATOS);

	free(initialBLock);
	free(restaurante->nombre);
	free(restaurante);

	return platos;
}

uint32_t* sindicato_api_guardar_pedido(void* pedido){
	t_nombre_y_id* pedidoRestaurante = pedido;

	uint32_t* opResult = malloc(sizeof(uint32_t));
	*opResult = SUCCESS_OPERATION;

	char* pedidoName;

	if(!sindicato_utils_verify_if_file_integrity_is_ok(pedidoRestaurante->nombre.nombre, TYPE_PEDIDO_INICIAL, pedidoRestaurante->id, &pedidoName )){

		log_error(sindicatoLog, "[FILESYSTEM] - El restaurante no existe o el pedido existe");

		*opResult = ERROR_OPERATION;

		if(pedidoName != NULL)
			free(pedidoName);
		free_struct_mensaje(pedidoRestaurante, GUARDAR_PEDIDO);

		return opResult;
	}

	char* pedidoString = internal_api_pedido_to_string("Pendiente","[]","[]","[]","0");

	int initialBlock = internal_api_write_block(pedidoString, NULL, MODE_ADD);
	if(initialBlock == ERROR_WRITE_BLOCK){

		log_error(sindicatoLog, "[FILESYSTEM] - Fallo la escritura del bloque, validar parametros enviados");

		*opResult = ERROR_OPERATION;

		free(pedidoString);
		if(pedidoName != NULL)
			free(pedidoName);
		free_struct_mensaje(pedidoRestaurante, GUARDAR_PEDIDO);

		return opResult;
	}

	/* Create "info" file */
	char* pedidoPath = sindicato_utils_build_file_full_path(sindicatoRestaurantePath, pedidoName, false, pedidoRestaurante->nombre.nombre);

	/* Build the key of the mutex RestauranteName + PedidoName */
	char* keyPedido = sindicato_utils_build_path(pedidoRestaurante->nombre.nombre, pedidoName);

	pthread_mutex_t* pedidoMutex = internal_utils_get_mutex(keyPedido);
	free(keyPedido);

	pthread_mutex_lock(pedidoMutex);
	internal_api_write_info_file(pedidoPath, initialBlock, pedidoString);
	pthread_mutex_unlock(pedidoMutex);
	log_info(sindicatoLog, "[FILESYSTEM] - Se creo el %s Pendiente [] [] [] 0", pedidoName);

	free(pedidoPath);
	free(pedidoString);
	free(pedidoName);
	/* *** UPDATE Restaurante *** */

	/* get the info from FS */
	pthread_mutex_t* restauranteMutex = internal_utils_get_mutex(pedidoRestaurante->nombre.nombre);
	pthread_mutex_lock(restauranteMutex);
	t_initialBlockInfo* initialBLock = internal_api_get_initial_block_info(pedidoRestaurante->nombre.nombre,NULL,TYPE_RESTAURANTE);

	t_restaurante_file* restauranteInfo = internal_api_read_blocks(initialBLock->initialBlock, initialBLock->stringSize);

	char* cantCocineros = string_itoa(restauranteInfo->cantidad_cocineros);
	char* posXY = string_new();
	string_append_with_format(&posXY, "[%d,%d]", restauranteInfo->posicion.x,restauranteInfo->posicion.x);
	char* afinidadCocinero = internal_api_list_to_string(restauranteInfo->afinidad_cocineros, 0);
	char* platos = internal_api_list_to_string(restauranteInfo->platos, 0);
	char* precioPlatos = internal_api_list_to_string(restauranteInfo->precios, 1);
	char* cantHornos = string_itoa(restauranteInfo->cantidad_hornos);
	int qtyPedidos = (int)restauranteInfo->cantidad_pedidos + 1;
	char* cantPedidos = string_itoa(qtyPedidos);

	list_destroy_and_destroy_elements(restauranteInfo->afinidad_cocineros, (void*) free_nombre);
	list_destroy_and_destroy_elements(restauranteInfo->platos, (void*) free_nombre);
	list_destroy(restauranteInfo->precios);
	free(restauranteInfo);

	char* restToSave = internal_api_restaurante_to_string(cantCocineros, posXY, afinidadCocinero, platos, precioPlatos, cantHornos, cantPedidos);

	free(cantCocineros);
	free(posXY);
	free(afinidadCocinero);
	free(platos);
	free(precioPlatos);
	free(cantHornos);
	free(cantPedidos);

	/* Create restaurante folder */
	char* restPath = sindicato_utils_build_path(sindicatoRestaurantePath, pedidoRestaurante->nombre.nombre);
	sindicato_utils_create_folder(restPath, true);
	free(restPath);

	int initialBlockRestaurante = internal_api_write_block(restToSave, initialBLock, MODE_UPDATE);
	if(initialBlockRestaurante == ERROR_WRITE_BLOCK){

		log_error(sindicatoLog, "[FILESYSTEM] - Fallo la escritura del bloque");

		*opResult = ERROR_OPERATION;

		pthread_mutex_unlock(restauranteMutex);

		free(restToSave);
		free_struct_mensaje(pedidoRestaurante, GUARDAR_PEDIDO);
		free(initialBLock);

		return opResult;
	}

	/* update "info" file */
	restPath = sindicato_utils_build_file_full_path(sindicatoRestaurantePath, pedidoRestaurante->nombre.nombre, true, NULL);
	internal_api_write_info_file(restPath, initialBlockRestaurante, restToSave);
	pthread_mutex_unlock(restauranteMutex);

	free(restPath);
	free(restToSave);
	free(initialBLock);
	free_struct_mensaje(pedidoRestaurante, GUARDAR_PEDIDO);

	return opResult;
}

uint32_t* sindicato_api_guardar_plato(void* pedido){
	m_guardarPlato* pedidoRequested = pedido;
	bool platoAdded = false;

	uint32_t* opResult = malloc(sizeof(uint32_t));
	*opResult = SUCCESS_OPERATION;

	char* pedidoName;
	if(!sindicato_utils_verify_if_file_integrity_is_ok(pedidoRequested->restaurante.nombre, TYPE_PEDIDO, pedidoRequested->idPedido, &pedidoName )){

		log_error(sindicatoLog, "[FILESYSTEM] - El restaurante no existe o el pedido no existe");

		*opResult = ERROR_OPERATION;

		if(pedidoName != NULL)
			free(pedidoName);
		free_struct_mensaje(pedidoRequested, GUARDAR_PLATO);

		return opResult;
	}

	/* Build the key of the mutex RestauranteName + PedidoName */
	char* keyPedido = sindicato_utils_build_path(pedidoRequested->restaurante.nombre, pedidoName);
	pthread_mutex_t* pedidoMutex = internal_utils_get_mutex(keyPedido);
	free(keyPedido);

	pthread_mutex_lock(pedidoMutex);
	/* get the info from FS */
	t_initialBlockInfo* initialBLock = internal_api_get_initial_block_info(pedidoName, pedidoRequested->restaurante.nombre, TYPE_PEDIDO);

	t_pedido_file* pedidoInfo = internal_api_read_blocks(initialBLock->initialBlock, initialBLock->stringSize);
	if(pedidoInfo->estado_pedido != PENDIENTE){

		log_error(sindicatoLog, "[FILESYSTEM] - El estado no es Pendiente");

		*opResult = ERROR_OPERATION;

		pthread_mutex_unlock(pedidoMutex);

		free(pedidoInfo);
		free(initialBLock);
		free_struct_mensaje(pedidoRequested, GUARDAR_PLATO);
		free(pedidoName);

		return opResult;
	}

	for(int i = 0; i < pedidoInfo->platos->elements_count; i++){
		t_nombre* plato = list_get(pedidoInfo->platos, i);

		if(string_equals_ignore_case(plato->nombre, pedidoRequested->comida.nombre)){
			uint32_t qtyPlatos = (uint32_t)list_remove(pedidoInfo->cantidad_platos, i);

			uint32_t newTotal = qtyPlatos + pedidoRequested->cantidad;

			list_add_in_index(pedidoInfo->cantidad_platos, i,(void*) newTotal);

			platoAdded = true;
		}
	}

	if(!platoAdded){
		list_add(pedidoInfo->platos, &pedidoRequested->comida);
		list_add(pedidoInfo->cantidad_platos, (void*)pedidoRequested->cantidad);
		list_add(pedidoInfo->cantidad_lista, 0);
	}

	char* platos = internal_api_list_to_string(pedidoInfo->platos, 0);
	char* cantPlatos = internal_api_list_to_string(pedidoInfo->cantidad_platos, 1);
	char* cantLista = internal_api_list_to_string(pedidoInfo->cantidad_lista, 1);

	pthread_mutex_t* restauranteMutex = internal_utils_get_mutex(pedidoRequested->restaurante.nombre);
	pthread_mutex_lock(restauranteMutex);
	/* get the info from FS */
	t_initialBlockInfo* initialBLockRestaurante = internal_api_get_initial_block_info(pedidoRequested->restaurante.nombre,NULL,TYPE_RESTAURANTE);
	pthread_mutex_unlock(restauranteMutex);
	t_restaurante_file* restauranteInfo = internal_api_read_blocks(initialBLockRestaurante->initialBlock, initialBLockRestaurante->stringSize);

	uint32_t newPrecio = pedidoInfo->precio_total;

	for(int i = 0; i < restauranteInfo->platos->elements_count; i++){
		t_nombre* plato = list_get(restauranteInfo->platos, i);
		if(string_equals_ignore_case(plato->nombre, pedidoRequested->comida.nombre)){
			uint32_t precio = (uint32_t)list_get(restauranteInfo->precios, i);

			newPrecio = newPrecio + precio * pedidoRequested->cantidad;
		}
	}

	internal_api_free_restaurante_file(restauranteInfo, GUARDAR_PLATO);

	free(initialBLockRestaurante);

	char* precioTotal = string_itoa((int)newPrecio);

	char* pedidoString = internal_api_pedido_to_string("Pendiente",platos, cantPlatos, cantLista, precioTotal);

	free(platos);
	free(cantPlatos);
	free(cantLista);
	free(precioTotal);

	internal_api_free_pedido(pedidoInfo, GUARDAR_PLATO);

	int initialBlockNumber = internal_api_write_block(pedidoString, initialBLock, MODE_UPDATE);
	if(initialBlockNumber == ERROR_WRITE_BLOCK){

		log_error(sindicatoLog, "[FILESYSTEM] - Fallo la escritura del bloque");

		*opResult = ERROR_OPERATION;

		pthread_mutex_unlock(pedidoMutex);

		free(pedidoString);
		free(initialBLock);
		free(pedidoInfo);
		free_struct_mensaje(pedidoRequested, GUARDAR_PLATO);
		free(pedidoName);

		return opResult;
	}

	/* Create "info" file */
	char* pedidoPath = sindicato_utils_build_file_full_path(sindicatoRestaurantePath, pedidoName, false, pedidoRequested->restaurante.nombre);

	internal_api_write_info_file(pedidoPath, initialBlockNumber, pedidoString);
	pthread_mutex_unlock(pedidoMutex);

	free(pedidoPath);
	free(pedidoString);
	free(initialBLock);
	free(pedidoName);
	free_struct_mensaje(pedidoRequested, GUARDAR_PLATO);

	return opResult;
}

uint32_t* sindicato_api_confirmar_pedido(void* pedido){
	t_nombre_y_id* pedidoRequested = pedido;

	uint32_t* opResult = malloc(sizeof(uint32_t));
	*opResult = SUCCESS_OPERATION;

	char* pedidoName;
	if(!sindicato_utils_verify_if_file_integrity_is_ok(pedidoRequested->nombre.nombre, TYPE_PEDIDO, pedidoRequested->id, &pedidoName )){

		log_error(sindicatoLog, "[FILESYSTEM] - El restaurante no existe o el pedido no existe");

		*opResult = ERROR_OPERATION;

		if(pedidoName != NULL)
			free(pedidoName);
		free_struct_mensaje(pedidoRequested, CONFIRMAR_PEDIDO);

		return opResult;
	}

	/* Build the key of the mutex RestauranteName + PedidoName */
	char* keyPedido = sindicato_utils_build_path(pedidoRequested->nombre.nombre, pedidoName);

	pthread_mutex_t* pedidoMutex = internal_utils_get_mutex(keyPedido);
	free(keyPedido);

	pthread_mutex_lock(pedidoMutex);
	/* get the info from FS */
	t_initialBlockInfo* initialBLock = internal_api_get_initial_block_info(pedidoName, pedidoRequested->nombre.nombre, TYPE_PEDIDO);

	t_pedido_file* pedidoInfo = internal_api_read_blocks(initialBLock->initialBlock, initialBLock->stringSize);

	if(pedidoInfo->estado_pedido != PENDIENTE){

		log_error(sindicatoLog, "[FILESYSTEM] - El estado no es Pendiente");

		*opResult = ERROR_OPERATION;

		pthread_mutex_unlock(pedidoMutex);

		free(pedidoInfo);
		free(initialBLock);
		free(pedidoName);
		free_struct_mensaje(pedidoRequested, CONFIRMAR_PEDIDO);

		return opResult;
	}

	char* platos = internal_api_list_to_string(pedidoInfo->platos, 0);
	char* cantPlatos = internal_api_list_to_string(pedidoInfo->cantidad_platos, 1);
	char* cantLista = internal_api_list_to_string(pedidoInfo->cantidad_lista, 1);
	char* precioTotal = string_itoa((int)pedidoInfo->precio_total);

	char* pedidoString = internal_api_pedido_to_string("Confirmado",platos, cantPlatos, cantLista, precioTotal);

	internal_api_free_pedido(pedidoInfo, CONFIRMAR_PEDIDO);

	free(platos);
	free(cantPlatos);
	free(cantLista);
	free(precioTotal);

	int initialBlockNumber = internal_api_write_block(pedidoString, initialBLock, MODE_UPDATE);
	if(initialBlockNumber == ERROR_WRITE_BLOCK){

		log_error(sindicatoLog, "[FILESYSTEM] - Fallo la escritura del bloque");

		*opResult = ERROR_OPERATION;

		pthread_mutex_unlock(pedidoMutex);

		free(pedidoString);
		free(initialBLock);
		free_struct_mensaje(pedidoRequested, CONFIRMAR_PEDIDO);

		return opResult;
	}

	/* Create "info" file */
	char* pedidoPath = sindicato_utils_build_file_full_path(sindicatoRestaurantePath, pedidoName, false, pedidoRequested->nombre.nombre);

	internal_api_write_info_file(pedidoPath, initialBlockNumber, pedidoString);
	pthread_mutex_unlock(pedidoMutex);

	free(pedidoPath);
	free(pedidoString);
	free(initialBLock);
	free(pedidoName);
	free_struct_mensaje(pedidoRequested, CONFIRMAR_PEDIDO);

	return opResult;
}

rta_obtenerPedido* sindicato_api_obtener_pedido(void* consultapedido){
	t_nombre_y_id* pedidoRequested = consultapedido;

	char* pedidoName;
	if(!sindicato_utils_verify_if_file_integrity_is_ok(pedidoRequested->nombre.nombre, TYPE_PEDIDO, pedidoRequested->id, &pedidoName )){

		log_error(sindicatoLog, "[FILESYSTEM] - El restaurante no existe o el pedido no existe");

		if(pedidoName != NULL)
			free(pedidoName);
		free_struct_mensaje(pedidoRequested, OBTENER_PEDIDO);

		return NULL;
	}

	/* Build the key of the mutex RestauranteName + PedidoName */
	char* keyPedido = sindicato_utils_build_path(pedidoRequested->nombre.nombre, pedidoName);
	pthread_mutex_t* pedidoMutex = internal_utils_get_mutex(keyPedido);
	free(keyPedido);

	pthread_mutex_lock(pedidoMutex);
	/* get the info from FS */
	t_initialBlockInfo* initialBLock = internal_api_get_initial_block_info(pedidoName, pedidoRequested->nombre.nombre, TYPE_PEDIDO);
	pthread_mutex_unlock(pedidoMutex);

	t_pedido_file* pedidoInfo = internal_api_read_blocks(initialBLock->initialBlock, initialBLock->stringSize);

	rta_obtenerPedido* pedido = malloc(sizeof(rta_obtenerPedido));
	pedido->infoPedidos = list_create();

	pedido->estadoPedido = pedidoInfo->estado_pedido;

	for(int i = 0; i < pedidoInfo->platos->elements_count; i++){
		t_elemPedido* pedidoElem = malloc(sizeof(t_elemPedido));

		t_nombre* nombre = list_get(pedidoInfo->platos, i);

		pedidoElem->comida.nombre = string_duplicate(nombre->nombre);
		pedidoElem->cantTotal = (uint32_t)list_get(pedidoInfo->cantidad_platos, i);
		pedidoElem->cantHecha = (uint32_t)list_get(pedidoInfo->cantidad_lista, i);

		list_add(pedido->infoPedidos, pedidoElem);
	}

	pedido->cantPedidos = pedidoInfo->platos->elements_count;


	internal_api_free_pedido(pedidoInfo, OBTENER_PEDIDO);
	free(pedidoName);
	free(initialBLock);
	free_struct_mensaje(pedidoRequested, OBTENER_PEDIDO);

	return pedido;
}

rta_obtenerRestaurante* sindicato_api_obtener_restaurante(void* restaurante){
	t_nombre* restauranteName = restaurante;

	if(!sindicato_utils_verify_if_exist(restauranteName->nombre, NULL, TYPE_RESTAURANTE)){

		log_error(sindicatoLog, "[FILESYSTEM] - El restaurante no existe");

		free(restauranteName->nombre);
		free(restauranteName);

		return NULL;
	}

	/* Initialize of restaurante structure */
	rta_obtenerRestaurante* rtaRestaurante = malloc(sizeof(rta_obtenerRestaurante));
	rtaRestaurante->recetas = list_create();

	pthread_mutex_t* restauranteMutex = internal_utils_get_mutex(restauranteName->nombre);
	pthread_mutex_lock(restauranteMutex);
	/* get the info from FS */
	t_initialBlockInfo* initialBLock = internal_api_get_initial_block_info(restauranteName->nombre,NULL,TYPE_RESTAURANTE);
	pthread_mutex_unlock(restauranteMutex);

	t_restaurante_file* restauranteInfo = internal_api_read_blocks(initialBLock->initialBlock, initialBLock->stringSize);

	rtaRestaurante->afinidades = list_duplicate(restauranteInfo->afinidad_cocineros);
	rtaRestaurante->cantAfinidades = rtaRestaurante->afinidades->elements_count;
	rtaRestaurante->posicion = restauranteInfo->posicion;

	for(int i = 0; i < restauranteInfo->platos->elements_count; i++){
		t_receta* recetaPrecio = malloc(sizeof(t_receta));

		t_nombre* n = list_get(restauranteInfo->platos, i);
		uint32_t* p = list_get(restauranteInfo->precios, i);

		recetaPrecio->receta.nombre = string_duplicate(n->nombre);
		recetaPrecio->precio = (uint32_t)p;

		list_add(rtaRestaurante->recetas, recetaPrecio);
	}

	rtaRestaurante->cantRecetas = rtaRestaurante->recetas->elements_count;

	rtaRestaurante->cantHornos = restauranteInfo->cantidad_hornos;
	rtaRestaurante->cantCocineros = restauranteInfo->cantidad_cocineros;
	rtaRestaurante->cantPedidos = restauranteInfo->cantidad_pedidos;

	internal_api_free_restaurante_file(restauranteInfo, OBTENER_RESTAURANTE);

	free(restauranteName->nombre);
	free(restauranteName);
	free(initialBLock);

	return rtaRestaurante;
}

uint32_t* sindicato_api_plato_listo(void* plato){
	m_platoListo* pedidoRequested = plato;
	uint32_t* opResult = malloc(sizeof(uint32_t));
	*opResult = SUCCESS_OPERATION;
	bool platoFound = false;

	char* pedidoName;
	if(!sindicato_utils_verify_if_file_integrity_is_ok(pedidoRequested->restaurante.nombre, TYPE_PEDIDO, pedidoRequested->idPedido, &pedidoName )){

		log_error(sindicatoLog, "[FILESYSTEM] - El restaurante no existe o el pedido no existe");

		if(pedidoName != NULL)
			free(pedidoName);
		free_struct_mensaje(pedidoRequested, PLATO_LISTO);

		return NULL;
	}

	/* Build the key of the mutex RestauranteName + PedidoName */
	char* keyPedido = sindicato_utils_build_path(pedidoRequested->restaurante.nombre, pedidoName);

	pthread_mutex_t* pedidoMutex = internal_utils_get_mutex(keyPedido);
	free(keyPedido);
	pthread_mutex_lock(pedidoMutex);
	/* get the info from FS */
	t_initialBlockInfo* initialBLock = internal_api_get_initial_block_info(pedidoName, pedidoRequested->restaurante.nombre, TYPE_PEDIDO);

	t_pedido_file* pedidoInfo = internal_api_read_blocks(initialBLock->initialBlock, initialBLock->stringSize);
	if(pedidoInfo->estado_pedido != CONFIRMADO){

		log_error(sindicatoLog, "[FILESYSTEM] - El estado no es Confirmado");

		*opResult = ERROR_OPERATION;

		pthread_mutex_unlock(pedidoMutex);

		free(initialBLock);
		internal_api_free_pedido(pedidoInfo, PLATO_LISTO);
		free(pedidoName);
		free_struct_mensaje(pedidoRequested, PLATO_LISTO);

		return opResult;
	}

	for(int i = 0; i < pedidoInfo->platos->elements_count; i++){
		t_nombre* plato = list_get(pedidoInfo->platos, i);

		if(string_equals_ignore_case(plato->nombre, pedidoRequested->comida.nombre)){
			uint32_t qtyPlatos = (uint32_t)list_remove(pedidoInfo->cantidad_lista, i);

			uint32_t newTotal = qtyPlatos + 1;

			list_add_in_index(pedidoInfo->cantidad_lista, i,(void*) newTotal);

			platoFound = true;
		}
	}

	if(!platoFound){

		log_error(sindicatoLog, "[FILESYSTEM] - El plato no pertenece al pedio");

		*opResult = ERROR_OPERATION;

		pthread_mutex_unlock(pedidoMutex);

		free(initialBLock);
		internal_api_free_pedido(pedidoInfo, PLATO_LISTO);
		free(pedidoName);
		free_struct_mensaje(pedidoRequested, PLATO_LISTO);

		return opResult;
	}

	char* platos = internal_api_list_to_string(pedidoInfo->platos, 0);
	char* cantPlatos = internal_api_list_to_string(pedidoInfo->cantidad_platos, 1);
	char* cantLista = internal_api_list_to_string(pedidoInfo->cantidad_lista, 1);
	char* precioTotal = string_itoa((int)pedidoInfo->precio_total);

	char* pedidoString = internal_api_pedido_to_string("Confirmado",platos, cantPlatos, cantLista, precioTotal);

	free(platos);
	free(cantPlatos);
	free(cantLista);
	free(precioTotal);

	int initialBlock = internal_api_write_block(pedidoString, initialBLock, MODE_UPDATE);
	if(initialBlock == ERROR_WRITE_BLOCK){

		log_error(sindicatoLog, "[FILESYSTEM] - Fallo la escritura del bloque");

		*opResult = ERROR_OPERATION;

		pthread_mutex_unlock(pedidoMutex);

		free(pedidoString);
		free(initialBLock);
		internal_api_free_pedido(pedidoInfo, PLATO_LISTO);
		free(pedidoName);
		free_struct_mensaje(pedidoRequested, PLATO_LISTO);

		return opResult;
	}

	/* Create "info" file */
	char* pedidoPath = sindicato_utils_build_file_full_path(sindicatoRestaurantePath, pedidoName, false, pedidoRequested->restaurante.nombre);

	internal_api_write_info_file(pedidoPath, initialBlock, pedidoString);
	pthread_mutex_unlock(pedidoMutex);

	free(pedidoPath);
	free(pedidoString);
	free(initialBLock);
	internal_api_free_pedido(pedidoInfo, PLATO_LISTO);
	free(pedidoName);
	free_struct_mensaje(pedidoRequested, PLATO_LISTO);

	return opResult;
}

rta_obtenerReceta* sindicato_api_obtener_receta(void* plato){
	t_nombre* recetaRequested = plato;

	if(!sindicato_utils_verify_if_file_integrity_is_ok(recetaRequested->nombre, TYPE_RECETA, 0, NULL )){

		log_error(sindicatoLog, "[FILESYSTEM] - La receta no existe en el FS");

		free(recetaRequested->nombre);
		free(recetaRequested);
		return NULL;
	}

	/* Initialize of receta structure */
	rta_obtenerReceta* receta = malloc(sizeof(rta_obtenerReceta));

	pthread_mutex_t* recetaMutex = internal_utils_get_mutex(recetaRequested->nombre);
	pthread_mutex_lock(recetaMutex);
	/* get the info from FS */
	t_initialBlockInfo* initialBLock = internal_api_get_initial_block_info(recetaRequested->nombre,NULL,TYPE_RECETA);
	pthread_mutex_unlock(recetaMutex);

	t_receta_file* recetaInfo = internal_api_read_blocks(initialBLock->initialBlock, initialBLock->stringSize);

	/* map values retrieved from FS */
	receta->pasos = list_duplicate(recetaInfo->pasos);
	receta->cantPasos = receta->pasos->elements_count;

	list_destroy(recetaInfo->pasos);
	free(recetaInfo);
	free(initialBLock);
	free(recetaRequested->nombre);
	free(recetaRequested);

	return receta;
}

uint32_t* sindicato_api_terminar_pedido(void* pedido){
	t_nombre_y_id* pedidoRequested = pedido;

	uint32_t* opResult = malloc(sizeof(uint32_t));
	*opResult = SUCCESS_OPERATION;

	char* pedidoName;
	if(!sindicato_utils_verify_if_file_integrity_is_ok(pedidoRequested->nombre.nombre, TYPE_PEDIDO, pedidoRequested->id, &pedidoName )){

		log_error(sindicatoLog, "[FILESYSTEM] - El restaurante no existe o el pedido no existe");

		*opResult = ERROR_OPERATION;

		if(pedidoName != NULL)
			free(pedidoName);
		free_struct_mensaje(pedidoRequested, TERMINAR_PEDIDO);
		return opResult;
	}

	/* Build the key of the mutex RestauranteName + PedidoName */
	char* keyPedido = sindicato_utils_build_path(pedidoRequested->nombre.nombre, pedidoName);

	pthread_mutex_t* pedidoMutex = internal_utils_get_mutex(keyPedido);
	free(keyPedido);
	pthread_mutex_lock(pedidoMutex);
	/* get the info from FS */
	t_initialBlockInfo* initialBLock = internal_api_get_initial_block_info(pedidoName, pedidoRequested->nombre.nombre, TYPE_PEDIDO);

	t_pedido_file* pedidoInfo = internal_api_read_blocks(initialBLock->initialBlock, initialBLock->stringSize);
	if(pedidoInfo->estado_pedido != CONFIRMADO){

		log_error(sindicatoLog, "[FILESYSTEM] - El estado no es Confirmado");

		*opResult = ERROR_OPERATION;

		pthread_mutex_unlock(pedidoMutex);

		free(initialBLock);
		internal_api_free_pedido(pedidoInfo, TERMINAR_PEDIDO);
		free_struct_mensaje(pedidoRequested, TERMINAR_PEDIDO);

		return opResult;
	}

	char* platos = internal_api_list_to_string(pedidoInfo->platos, 0);
	char* cantPlatos = internal_api_list_to_string(pedidoInfo->cantidad_platos, 1);
	char* cantLista = internal_api_list_to_string(pedidoInfo->cantidad_lista, 1);
	char* precioTotal = string_itoa((int)pedidoInfo->precio_total);

	char* pedidoString = internal_api_pedido_to_string("Terminado",platos, cantPlatos, cantLista, precioTotal);

	free(platos);
	free(cantPlatos);
	free(cantLista);
	free(precioTotal);
	internal_api_free_pedido(pedidoInfo, TERMINAR_PEDIDO);

	int initialBlockNumber = internal_api_write_block(pedidoString, initialBLock, MODE_UPDATE);
	if(initialBlockNumber == ERROR_WRITE_BLOCK){

		log_error(sindicatoLog, "[FILESYSTEM] - Fallo la escritura del bloque");

		*opResult = ERROR_OPERATION;

		pthread_mutex_unlock(pedidoMutex);

		free(pedidoString);
		free(initialBLock);
		free_struct_mensaje(pedidoRequested, TERMINAR_PEDIDO);

		return opResult;
	}

	/* Create "info" file */
	char* pedidoPath = sindicato_utils_build_file_full_path(sindicatoRestaurantePath, pedidoName, false, pedidoRequested->nombre.nombre);

	internal_api_write_info_file(pedidoPath, initialBlockNumber, pedidoString);
	pthread_mutex_unlock(pedidoMutex);

	free(pedidoPath);
	free(pedidoString);
	free(initialBLock);
	free(pedidoName);
	free_struct_mensaje(pedidoRequested, TERMINAR_PEDIDO);

	return opResult;
}

/* Main functions */
void sindicato_api_afip_initialize(){

	internal_api_createFileSystemFolders();

	internal_api_initialize_metadata();

	internal_api_bitmap_create();

	internal_api_initialize_blocks();

	mutexDictionary = dictionary_create();
	pthread_mutex_init(&dictionary_mtx, NULL);
}
