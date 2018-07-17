#ifndef UTILIDADES_H
#define UTILIDADES_H

#include "configuracion.h"
#include "mensajeria.h"

typedef struct jugador {
	int socket;
	char apodo[TAMANIO_DE_NOMBRE];
	int carta_jugada;
	bool se_quedo_sin_cartas;
} jugador;

/**
 * Envia un mensaje cpn codigo y contenido a todos los jugadores.
 * */
void broadcast( int codigo_de_mensaje, char* contenido, jugador* receptores, int cantidad_de_receptores );

/**
 * Envia un mensaje con solamente un codigo a todos los jugadores.
 * */
void broadcast_liviano( int codigo_de_mensaje, jugador* receptores, int cantidad_de_receptores );

/**
 * Obtiene el nombre de un jugador a traves de un mensaje
 * */
void extraer_apodo_de_mensaje( char** mensaje, char* apodo);

/**
 * Devuelve un entero menor que la cota superior solicitada.
 * */
int obtener_numero_aleatorio( int cota_superior );

/**
 * Determina si un valor esta contenido en un arreglo
 * */
bool arreglo_contiene( int* pajar, int aguja, int tamanio_pajar );

#endif