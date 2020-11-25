#include "SindicatoConsole.h"

/* ********************************** PRIVATE FUNCTIONS ********************************** */

void internal_console_free(char* read, char** command){

	int position = 0;

	while(command[position] != NULL){
		free(command[position]);
		position++;
	}

	free(command);
	free(read);
}

bool internal_console_validate_arguments(char** arguments, command_type_id commandType){

	int position = 0;

	while(arguments[position] != NULL){
		position++;
	}

	if(position == QTY_CREAR_RESTAURANTE && commandType == TYPE_CREAR_RESTAURANTE)
		return true;

	if(position == QTY_CREAR_RECETA && commandType == TYPE_CREAR_RECETA)
		return true;

	if(position == QTY_HELP && commandType == TYPE_HELP)
		return true;

	return false;
}

void internal_console_execute_command(char** commandLine){

	/* Validate if the commandLine is valid */
	if(!string_equals_ignore_case(commandLine[0],COMMAND_CREAR_RESTAURANTE)
			&& !string_equals_ignore_case(commandLine[0],COMMAND_CREAR_RECETA)
			&& !string_equals_ignore_case(commandLine[0],COMMAND_HELP)){

		printf("%s%s\n",commandLine[0],ERROR_COMMAND);
		return;
	}

	/* Validate if the arguments are valid */
	if(!internal_console_validate_arguments(commandLine, TYPE_CREAR_RESTAURANTE)
			&& !internal_console_validate_arguments(commandLine, TYPE_CREAR_RECETA)
			&& !internal_console_validate_arguments(commandLine, TYPE_HELP)){

		printf("%s %s\n",ERROR_ARGUMENTS,commandLine[0]);
		return;
	}

	if(string_equals_ignore_case(commandLine[0],COMMAND_CREAR_RESTAURANTE)){
		sindicato_api_crear_restaurante(commandLine[1],commandLine[2],commandLine[3],commandLine[4],commandLine[5],commandLine[6],commandLine[7]);
		return;

	}

	if(string_equals_ignore_case(commandLine[0],COMMAND_CREAR_RECETA)){
		sindicato_api_crear_receta(commandLine[1],commandLine[2],commandLine[3]);
		return;
	}

	if(string_equals_ignore_case(commandLine[0],COMMAND_HELP)){
		printf("*** Comandos validos ***\n");
		printf("CrearRestaurante [NOMBRE] [CANTIDAD_COCINEROS] [POSICION] [AFINIDAD_COCINEROS] [PLATOS] [PRECIO_PLATOS] [CANTIDAD_HORNOS]\n");
		printf("CrearReceta [NOMBRE] [PASOS] [TIEMPO_PASOS]\n");
		printf("exit\n");
		printf("help\n");
		printf("Type help for more comands\n");
		return;
	}
}

void internal_console_init(){

	char* read;

	while(true){
		read = readline(">");

		if(read) add_history(read);

		if(string_is_empty(read)){
			free(read);
			continue;
		}

		char** splitted_command = string_split(read," ");

		if(string_equals_ignore_case(splitted_command[0],COMMAND_EXIT)){
			internal_console_free(read, splitted_command);
			break;
		}
		
		internal_console_execute_command(splitted_command);

		internal_console_free(read, splitted_command);
	}
}

/* ********************************** PUBLIC  FUNCTIONS ********************************** */

void sindicato_console_initialize(){

	printf("Initializing console\n");
	printf("Type help for more comands\n");

	internal_console_init();
}
