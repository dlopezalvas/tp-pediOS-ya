#ifndef SINDICATOUTILS_H_
#define SINDICATOUTILS_H_

#include <stdio.h>
#include <stdlib.h>

/* Valid commands */
#define COMMAND_CREAR_RESTAURANTE "CrearRestaurante"
#define COMMAND_CREAR_RECETA "CrearReceta"
#define COMMAND_EXIT "exit"

/* Commands lenght */
#define LENGHT_CREAR_RESTAURANTE 16
#define LENGHT_CREAR_RECETA 11
#define LENGHT_EXIT 4

/* Error message */
#define ERROR_COMMAND ": Command not found"
#define ERROR_ARGUMENTS "Error: Invalid arguments for command: "

typedef enum{
	NO_TIENE_ID = 0,
	ID_AL_FINAL = 1,
	ID_AL_PRINCIPIO = 2,
}tipo_id;




void sindicato_initialize();

#endif /* SINDICATOUTILS_H_ */
