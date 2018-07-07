#include "mensajeria.h"


void enviar_mensaje_pre_formateado(char* mensaje_pre_formateado, int socket) {
	if (send(socket, mensaje_pre_formateado, TAMANIO_MENSAJE, 0) == -1) {
		perror("Hubo al tratar de enviar un mensaje al servidor.");
		exit(EXIT_FAILURE);
	}
}

void enviar_mensaje(int codigo_de_mensaje, const char* contenido, int socket) {
	char mensaje[TAMANIO_MENSAJE];
	sprintf(mensaje, "%d %s", codigo_de_mensaje, contenido);
	enviar_mensaje_pre_formateado(mensaje, socket);
}

void enviar_mensaje_sin_cuerpo(int codigo_de_mensaje, int socket) {
	char mensaje[TAMANIO_MENSAJE];
	sprintf(mensaje, "%d", codigo_de_mensaje);
	enviar_mensaje_pre_formateado(mensaje, socket);
}

void enviar_mensaje_numerico(int codigo_de_mensaje, int contenido, int socket) {
	char mensaje[TAMANIO_MENSAJE];
	sprintf(mensaje, "%d %d", codigo_de_mensaje, contenido);
	enviar_mensaje_pre_formateado(mensaje, socket);
}

int extraer_codigo_de_mensaje(char** mensaje) {
	return atoi(strtok_r(*mensaje, " ", mensaje));
}

int decodificar_contenido_de_mensaje(char** contenido_crudo, int* contenido_decodificado, int cantidad_de_elementos) {
	int i;

	for (i = 0; i < cantidad_de_elementos; i++) {
		char* token = strtok_r(*contenido_crudo, " ", contenido_crudo );
		
		if (token == NULL) {
			return i;
		}

		*(contenido_decodificado + i) = atoi(token);
	}

	return i;
}

