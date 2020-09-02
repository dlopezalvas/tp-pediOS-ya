#include "SindicatoUtils.h"
#include <commons/string.h>
#include <readline/readline.h>

/* ********************************** PRIVATE FUNCTIONS ********************************** */

void execute(char** command){
	//TODO: Definir el llamado de los comandos
	//TODO: Crear sindicato API donde van a ir todos los mensajes y funciones de file system
}

bool validate_arguments(char** arguments, int command){
	return true;
}

void execute_command(char** command){

	if((strncmp(command[0],COMMAND_CREAR_RESTAURANTE,LENGHT_CREAR_RESTAURANTE) != 0) && (strncmp(command[0],COMMAND_CREAR_RECETA,LENGHT_CREAR_RECETA) != 0 ))
		printf("%s%s\n",command[0],ERROR_COMMAND);

	if(strlen(command[0]) == LENGHT_CREAR_RESTAURANTE){
		if(!validate_arguments(command, 1))
			printf("%s %s\n",ERROR_ARGUMENTS,command[0]);

		execute(command);
	}

	if(strlen(command[0]) == LENGHT_CREAR_RECETA){
		if(!validate_arguments(command, 2))
			printf("%s %s\n",ERROR_ARGUMENTS,command[0]);

		execute(command);
	}
}

void console_init(){

	char* read;

	while(true){
		read = readline(">");

		if(strncmp(read, COMMAND_EXIT, LENGHT_EXIT) == 0 && strlen(read) == LENGHT_EXIT){
			break;
		}

		printf("%s\n", read);

		char** splitted_command = string_split(read," ");

		execute_command(splitted_command);
			
		free(read);
	}
}

/* ********************************** PUBLIC  FUNCTIONS ********************************** */

void sindicato_initialize(){
	printf("Initializing\n");

	console_init();
}

