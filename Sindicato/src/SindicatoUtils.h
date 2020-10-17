#ifndef SINDICATOUTILS_H_
#define SINDICATOUTILS_H_

#include <stdio.h>
#include <stdlib.h>

#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>

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

/* ********************************** PUBLIC  FUNCTIONS ********************************** */

t_log* sindicato_utils_iniciar_debug_logger(t_config* config);

#endif /* SINDICATOUTILS_H_ */
