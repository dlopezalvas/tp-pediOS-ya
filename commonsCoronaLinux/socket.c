#include "socket.h"


int iniciar_servidor (char* ip, int puerto){
	struct sockaddr_in direccion_servidor;

	direccion_servidor.sin_family = AF_INET;
	direccion_servidor.sin_addr.s_addr = INADDR_ANY;
	direccion_servidor.sin_port = htons(puerto);

	int servidor = socket(AF_INET, SOCK_STREAM,0);

	//para poder probar que ande sin tener que esperar 2min
	int activado = 1;
	setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	if(bind(servidor, (void*) &direccion_servidor, sizeof(direccion_servidor)) !=0){
		perror("Fallo el bind");
	}


	listen(servidor,SOMAXCONN); //flag para que tome el maximo posible de espacio



	return servidor;

}



int iniciar_cliente(char* ip, int puerto){
	struct sockaddr_in direccion_servidor;

	direccion_servidor.sin_family = AF_INET;
	direccion_servidor.sin_addr.s_addr = inet_addr(ip);
	direccion_servidor.sin_port = htons(puerto);

	int cliente = socket(AF_INET, SOCK_STREAM, 0);

	if(connect(cliente, (void*) &direccion_servidor, sizeof(direccion_servidor)) !=0){
		perror("No se pudo conectar");
		return -1;
	}

	return cliente;
}

void* recibir_mensaje(int socket_cliente, int* size)
{
	void * buffer;

	int aux_size = 0;

	recv(socket_cliente, &aux_size, sizeof(int), 0);
	buffer = malloc(aux_size);
	recv(socket_cliente, buffer, aux_size, 0);

	*size = aux_size;

	return buffer;
}

void liberar_conexion(int socket_cliente){
	close(socket_cliente);
}


