#include "server_utils.h"

void broadcast(int msg_code, char* payload, player* recipients, int rcp_count) {
	char msg[TAMANIO_MENSAJE];
	sprintf(msg, "%d %s", msg_code, payload);
	int i;
	for (i = 0; i < rcp_count; i++) {
		if (recipients[i].socket != 0) {
			enviar_mensaje_pre_formateado(msg, recipients[i].socket);
		}
	}
}

void broadcast_light(int msg_code, player* recipients, int rcp_count) {
	char msg[TAMANIO_MENSAJE];
	sprintf(msg, "%d", msg_code);
	int i;
	for (i = 0; i < rcp_count; i++) {
		if (recipients[i].socket != 0) {
			enviar_mensaje_pre_formateado(msg, recipients[i].socket);
		}
	}
}

void extract_player_nickname(char** msg, char* nickname) {
	sprintf(nickname,"%s", strtok_r(*msg, " ", msg));
}

int rand_range(int upper_limit) {
	return (int) (( (double) upper_limit / RAND_MAX) * rand());
}

bool array_contains(int* haystack, int needle, int length) {
	int* array_ptr = haystack;
	for (; (array_ptr - haystack) < length; array_ptr++) {
		if (*array_ptr == needle) {
			return TRUE;
		}
	}
	return FALSE;
}
