#ifndef SINDICATOUTILS_H_
#define SINDICATOUTILS_H_

#include "SindicatoApi.h"

#include <commons/string.h>
#include <readline/readline.h>
#include <readline/history.h>

/* Valid commands */
#define COMMAND_CREAR_RESTAURANTE "CrearRestaurante"
#define COMMAND_CREAR_RECETA "CrearReceta"
#define COMMAND_EXIT "exit"
#define COMMAND_HELP "help"

/* Arguments quantity */
#define QTY_CREAR_RESTAURANTE 8
#define QTY_CREAR_RECETA 4

/* Error message */
#define ERROR_COMMAND ": Command not found"
#define ERROR_ARGUMENTS "Error - Invalid arguments for command: "

typedef enum{
	TYPE_CREAR_RESTAURANTE = 1,
	TYPE_CREAR_RECETA = 2,
}command_type_id;

/* ********************************** PUBLIC  FUNCTIONS ********************************** */

void sindicato_initialize();

#endif /* SINDICATOUTILS_H_ */
