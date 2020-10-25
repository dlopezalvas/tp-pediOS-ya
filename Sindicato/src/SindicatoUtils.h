#ifndef SINDICATOUTILS_H_
#define SINDICATOUTILS_H_

#include <sys/stat.h>

#include "../commonsCoronaLinux/logs.h"
#include "../commonsCoronaLinux/socket.h"
#include "../commonsCoronaLinux/utils.h"

#define SINDICATO_PATH_CONFIG "/home/utnso/workspace/tp-2020-2c-CoronaLinux/Sindicato/sindicato.config"

typedef struct{
	int socket;
	t_mensaje* message;
}t_responseMessage;

/* Global variables */
t_log* sindicatoLog;
t_log* sindicatoDebugLog;
t_config* sindicatoConfig;
uint32_t sindicatoProcessId;
int sindicatoPort;
char* sindicatoMountPoint;

/* ********************************** PUBLIC  FUNCTIONS ********************************** */

t_log* sindicato_utils_iniciar_debug_logger(t_config* config);
void sindicato_utils_create_folder(char* path, bool logsFolder);
char* sindicato_utils_build_path(char* path, char* toAppend);

#endif /* SINDICATOUTILS_H_ */
