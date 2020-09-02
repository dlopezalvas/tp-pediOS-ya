#ifndef socket_h
#define socket_h

#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "utils.h"
#include "serializacion.h"

#include <arpa/inet.h>//inet_addr
#include <pthread.h>


int iniciar_cliente(char *ip, int puerto);
int iniciar_servidor(char* ip, int puerto);
void* recibir_mensaje(int socket_cliente, int* size);
void liberar_conexion(int socket_cliente);



#endif /* socket_h */
