#include "SindicatoUtils.h"

/* ********************************** PUBLIC  FUNCTIONS ********************************** */

t_log* sindicato_utils_iniciar_debug_logger(t_config* config){
	char* nombre_archivo = config_get_string_value(config,"DEBUG_LOG_FILE");
	char* nombre_aplicacion = config_get_string_value(config,"DEBUG_NOMBRE_APLICACION");
	int log_consola = config_get_int_value(config,"DEBUG_LOG_CONSOLA");

	t_log* logger = log_create(nombre_archivo,nombre_aplicacion,log_consola,LOG_LEVEL_INFO);
	return logger;
}

void sindicato_utils_create_folder(char* path, bool recordLog){
	struct stat st = {0};

	/* Validate if the folder exists to create the folder */
	if(stat(path, &st) == -1){
		if(mkdir(path,0777) == 0){
			if(recordLog)
				log_info(sindicatoDebugLog, "[FILESYSTEM] - Carpeta creada: %s",path);
		}
	}else{
		if(recordLog)
			log_info(sindicatoDebugLog, "[FILESYSTEM] - Carpeta existente: %s", path);
	}
}

char* sindicato_utils_build_path(char* path, char* toAppend){
	char* pathBuilded = string_duplicate(path);
	string_append(&pathBuilded, toAppend);
	return pathBuilded;
}

char* sindicato_utils_build_file_full_path(char* path, char* fname, bool isRestaurante, char* restaurateOfPedido){

	/* Result = "fileName.AFIP" */
	char* fileName = string_duplicate(fname);
	string_append(&fileName, ".");
	string_append(&fileName, metadataFS->magic_number);

	/* path */
	char* folderPath = string_duplicate(path);
	if(isRestaurante){
		string_append(&folderPath, fname);
		string_append(&folderPath, "/");
	} else {
		if(restaurateOfPedido != NULL){
			string_append(&folderPath, restaurateOfPedido);
			string_append(&folderPath, "/");
		}
	}

	/* path + file name + extension */
	char* fullPath = sindicato_utils_build_path(folderPath, fileName);

	free(fileName);
	free(folderPath);

	return fullPath;
}

char* sindicato_utils_build_block_path(int blockNumber){
	char* fileExtension = sindicato_utils_build_path(".", metadataFS->magic_number);
	char* blockNumberString = string_itoa(blockNumber);
	char* fileName = sindicato_utils_build_path(blockNumberString, fileExtension);

	char* filePath = sindicato_utils_build_path(sindicatoBlocksPath, "/");
	char* filePathComplete = sindicato_utils_build_path(filePath, fileName);

	free(fileExtension);
	free(blockNumberString);
	free(fileName);
	free(filePath);

	return filePathComplete;
}

char* sindicato_utils_build_pedido_name(uint32_t idPedido){

	char* pedidoName = string_duplicate("Pedido");
	char* pedidoNumberString = string_itoa((int)idPedido);

	string_append(&pedidoName, pedidoNumberString);

	return pedidoName;
}

bool sindicato_utils_verify_if_file_exist(char* path){

	if(access(path, F_OK) != -1){
		log_info(sindicatoDebugLog, "[FILESYSTEM] - Existe el path %s", path);
		return true;
	}else{
		log_error(sindicatoDebugLog,  "[FILESYSTEM] - No existe el path %s", path);
		return false;
	}
}

bool sindicato_utils_verify_if_exist(char* fileName, char* restauranteOfPedido, file_type fileType){

	char* filePath;

	switch(fileType){
		case(TYPE_RECETA):
			filePath = sindicato_utils_build_file_full_path(sindicatoRecetaPath, fileName, false, NULL);
			break;
		case(TYPE_PEDIDO):
			filePath = sindicato_utils_build_file_full_path(sindicatoRestaurantePath, fileName, false, restauranteOfPedido);
			break;
		case(TYPE_RESTAURANTE):
			filePath = sindicato_utils_build_file_full_path(sindicatoRestaurantePath, fileName, true, NULL);
			break;
		default:
			break;
	}

	if(sindicato_utils_verify_if_file_exist(filePath)){
		free(filePath);
		return true;
	} else {
		free(filePath);
		return false;
	}
}

bool sindicato_utils_verify_if_file_integrity_is_ok(char* name, file_type fileType, uint32_t idPedido, char** pedidoName){

	bool result = false;

	if(fileType == TYPE_RECETA){

		/* Receta must exist in FS to be OK */
		if(sindicato_utils_verify_if_exist(name, NULL, TYPE_RECETA))
			result = true;
	}

	if(fileType == TYPE_RECETA_INICIAL){

		/* Receta must exist in FS to be OK */
		if(!sindicato_utils_verify_if_exist(name, NULL, TYPE_RECETA))
			result = true;
	}

	if(fileType == TYPE_RESTAURANTE_INICIAL){

		/* Restaurante must not exist in FS to be OK */
		if(!sindicato_utils_verify_if_exist(name, NULL, TYPE_RESTAURANTE))
			result = true;
	}

	if(fileType == TYPE_RESTAURANTE || fileType == TYPE_PEDIDO || fileType == TYPE_PEDIDO_INICIAL){

		/* Restaurante must exist in FS to be OK */
		if(sindicato_utils_verify_if_exist(name, NULL, TYPE_RESTAURANTE)){
			result = true;
		}

		if(!result)
			return result;


		if(fileType == TYPE_PEDIDO || fileType == TYPE_PEDIDO_INICIAL){
			*pedidoName = sindicato_utils_build_pedido_name(idPedido);

			/* Fist Pedido must no exist in FS to be OK */
			if(fileType == TYPE_PEDIDO_INICIAL){
				if(!sindicato_utils_verify_if_exist(*pedidoName, name, TYPE_PEDIDO)){
					result = true;
				} else{
					result = false;
				}
			}

			/* Pedido must exist in FS to be OK */
			if(fileType == TYPE_PEDIDO){
				if(sindicato_utils_verify_if_exist(*pedidoName, name, TYPE_PEDIDO)){
					result = true;
				} else {
					result = false;
				}
			}

		}
	}

	return result;
}

pthread_mutex_t* internal_utils_get_mutex(char* key){
	pthread_mutex_t* fileMutex = malloc(sizeof(pthread_mutex_t));

	pthread_mutex_lock(&dictionary_mtx);

	if(dictionary_has_key(mutexDictionary, key)){
		fileMutex = dictionary_get(mutexDictionary, key);

	} else {
		pthread_mutex_init(fileMutex, NULL);
		dictionary_put(mutexDictionary, key, fileMutex);
	}
	pthread_mutex_unlock(&dictionary_mtx);

	return fileMutex;
}

void sindicato_utils_free_memory_message(t_responseMessage* responseMessage){

	free_struct_mensaje(responseMessage->message->parametros, responseMessage->message->tipo_mensaje);

	free(responseMessage->message);
	free(responseMessage);
}
