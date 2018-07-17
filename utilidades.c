#include "utilidades.h"

void broadcast( int codigo_de_mensaje, char* contenido, jugador* receptores, int cantidad_de_receptores ) {
	char mensaje[TAMANIO_MENSAJE];
	sprintf(mensaje, "%d %s", codigo_de_mensaje, contenido);
	int i;

	for (i = 0; i < cantidad_de_receptores; i++) {
		if (receptores[i].socket != 0) {
			enviar_mensaje_pre_formateado(mensaje, receptores[i].socket);
		}
	}
}

void broadcast_liviano( int codigo_de_mensaje, jugador* receptores, int cantidad_de_receptores ) {
	char mensaje[TAMANIO_MENSAJE];
	sprintf(mensaje, "%d", codigo_de_mensaje);
	int i;

	for (i = 0; i < cantidad_de_receptores; i++) {
		if (receptores[i].socket != 0) {
			enviar_mensaje_pre_formateado(mensaje, receptores[i].socket);
		}
	}
}

void extraer_apodo_de_mensaje(char** mensaje, char* apodo) {
	sprintf(apodo,"%s", strtok_r(*mensaje, " ", mensaje));
}

int obtener_numero_aleatorio(int cota_superior) {
	return (int) (( (double) cota_superior / RAND_MAX) * rand());
}

bool arreglo_contiene(int* pajar, int aguja, int tamanio_pajar) {
	int* array_ptr = pajar;

	for (; (array_ptr - pajar) < tamanio_pajar; array_ptr++) {
		if (*array_ptr == aguja) {
			return TRUE;
		}
	}
	
	return FALSE;
}
