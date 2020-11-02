#include "socket.h"


int iniciar_servidor (int puerto){
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
		return -1;
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

void* recibir_mensaje(int socket_cliente, int* error){
    void * buffer;
    int aux_size = 0;
    int _recv;

    *error = 0;
    _recv = recv(socket_cliente, &aux_size, sizeof(int), 0);
    if (_recv == 0 || _recv == -1) {
        *error = 1;
        return NULL;
    }
    buffer = malloc(aux_size);
    if(aux_size != 0){
        _recv = recv(socket_cliente, buffer, aux_size, 0);
        if (_recv == 0 || _recv == -1) {
            *error = 1;
            return NULL;
        }
    }
    // *size = aux_size;
    return buffer;
}

void enviar_mensaje(t_mensaje* mensaje, int socket){

	t_paquete* paquete = malloc(sizeof(t_paquete));

	t_buffer* buffer_cargado = cargar_buffer(mensaje);

	paquete -> buffer = buffer_cargado;

	paquete -> codigo_operacion = mensaje -> tipo_mensaje;

	paquete->id = mensaje->id;

	int bytes = 0;

	void* a_enviar = serializar_paquete(paquete, &bytes);

	send(socket,a_enviar,bytes,0);

	free(a_enviar);
	if(op_code_to_struct_code(mensaje->tipo_mensaje)!= STRC_MENSAJE_VACIO) free(paquete -> buffer->stream);
	free(paquete->buffer);
	free(paquete);

}

void liberar_conexion(int socket_cliente){
	close(socket_cliente);
}

