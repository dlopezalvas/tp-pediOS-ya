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
				log_info(sindicatoDebugLog, "[FILESYSTEM] Carpeta creada: %s",path);
		}
	}else{
		if(recordLog)
			log_info(sindicatoDebugLog, "[FILESYSTEM] Carpeta existente: %s", path);
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

bool sindicato_utils_verify_if_file_exist(char* path){

	if(access(path, F_OK) != -1){
		log_info(sindicatoDebugLog, "[FILESYSTEM] - No existe el path %s", path);
		return true;
	}else{
		log_error(sindicatoDebugLog,  "[FILESYSTEM] - Existe el path %s", path);
		return false;
	}
}

bool sindicato_utils_verify_if_exist(char* fileName, file_type fileType){

	char* FilePath = sindicato_utils_build_file_full_path(sindicatoRestaurantePath, fileName, true, NULL);

	switch(fileType){
		case(TYPE_RECETA):
				//Provisorio
				return true;
		case(TYPE_PEDIDO):
				//Provisorio
				return true;
		case(TYPE_RESTAURANTE):

			if(sindicato_utils_verify_if_file_exist(FilePath)){
				free(FilePath);
				return true;
			} else {
				free(FilePath);
				return false;
			}
		default:
			//Provisorio
			return true;
	}

}

void sindicato_utils_free_memory_message(t_responseMessage* responseMessage){

	/* Vallidate which message shiuld be free */
	switch(responseMessage->message->tipo_mensaje){
		case RTA_CONSULTAR_PLATOS:
			free_restaurante_y_plato(responseMessage->message->parametros);
			break;
		case RTA_GUARDAR_PEDIDO:
			free_id_o_confirmacion(responseMessage->message->parametros);
			break;
		case RTA_GUARDAR_PLATO:
			free_id_o_confirmacion(responseMessage->message->parametros);
			break;
		case RTA_CONFIRMAR_PEDIDO:
			free_id_o_confirmacion(responseMessage->message->parametros);
			break;
		case RTA_OBTENER_PEDIDO:
			free_rta_obtener_pedido(responseMessage->message->parametros);
			break;
		case RTA_OBTENER_RESTAURANTE:
			free_rta_obtener_restaurante(responseMessage->message->parametros);
			break;
		case RTA_PLATO_LISTO:
			free_id_o_confirmacion(responseMessage->message->parametros);
			break;
		case RTA_OBTENER_RECETA:
			free_rta_obtener_receta(responseMessage->message->parametros);
			break;
		case RTA_TERMINAR_PEDIDO:
			free_id_o_confirmacion(responseMessage->message->parametros);
			break;
		default:
			break;
	}

	free(responseMessage->message);
	free(responseMessage);
}
