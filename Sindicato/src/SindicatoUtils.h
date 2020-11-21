#ifndef SINDICATOUTILS_H_
#define SINDICATOUTILS_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <math.h>

#include <commons/bitarray.h>

#include "../commonsCoronaLinux/logs.h"
#include "../commonsCoronaLinux/socket.h"
#include "../commonsCoronaLinux/utils.h"

#define SINDICATO_PATH_CONFIG "/home/utnso/workspace/tp-2020-2c-CoronaLinux/Sindicato/sindicato.config"

typedef struct{
	int socket;
	t_mensaje* message;
}t_responseMessage;

typedef struct{
	uint32_t block_size;
	uint32_t blocks;
	char* magic_number;
}t_metadata;

/* Global variables */

t_log* sindicatoLog;
t_log* sindicatoDebugLog;

t_config* sindicatoConfig;
t_config* metadataConfig;

int sindicatoPort;
char* sindicatoMountPoint;
char* sindicatoMetadataPath;
char* sindicatoBlocksPath;
char* sindicatoRecetaPath;
char* sindicatoRestaurantePath;

uint32_t sindicatoProcessId;

t_metadata* metadataFS;
t_bitarray* bitarray;

pthread_mutex_t bitarray_mtx;

/* ********************************** PUBLIC  FUNCTIONS ********************************** */

t_log* sindicato_utils_iniciar_debug_logger(t_config* config);
void sindicato_utils_create_folder(char* path, bool logsFolder);
char* sindicato_utils_build_path(char* path, char* toAppend);
char* sindicato_utils_build_file_full_path(char* path, char* name, bool isRestaurante, char* restaurateOfPedido);
char* sindicato_utils_build_block_path(int blockNumber);
bool sindicato_utils_verify_if_file_exist(char* path);
void sindicato_utils_free_memory_message(t_responseMessage* responseMessage);

#endif /* SINDICATOUTILS_H_ */
