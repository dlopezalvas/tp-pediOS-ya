#ifndef SINDICATOCONSOLE_H_
#define SINDICATOCONSOLE_H_

#include "SindicatoApi.h"

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
#define QTY_HELP 1

/* Error message */
#define ERROR_COMMAND ": Command not found"
#define ERROR_ARGUMENTS "Error - Invalid arguments for command: "

typedef enum{
	TYPE_CREAR_RESTAURANTE = 1,
	TYPE_CREAR_RECETA = 2,
	TYPE_HELP = 3,
}command_type_id;

/* ********************************** PUBLIC  FUNCTIONS ********************************** */

void sindicato_console_initialize();

#endif /* SINDICATOCONSOLE_H_ */
