#ifndef MENSAJERIA_H
#define MENSAJERIA_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include "config.h"

#define TAMANIO_MENSAJE 82 //the maximum size a message could ever have

/**
 * Envia un mensaje previamente formateado a un socket.
 * */
void enviar_mensaje_pre_formateado( char* mensaje, int socket);

/**
 * Formatea y envia un mensaje usando el codigo y texto elegidos, a un socket.
 * */
void enviar_mensaje( int codigo_de_mensaje, const char* contenido, int socket);

/**
 * Formatea y envia un mensaje que solo contiene un codigo a un socket.
 * */
void enviar_mensaje_sin_cuerpo( int codigo_de_mensaje, int socket);

/**
 * Formatea y envia un mensaje usando el codigo y numero elegidos, a un socket.
 * */
void enviar_mensaje_numerico( int codigo_de_mensaje, int contenido, int socket);

/**
 * Extrae el codigo de un mensaje y lo devuelve.
 * */
int extraer_codigo_de_mensaje( char** mensaje );

/** *
* Decoficia un mensaje y devuelve el contenido y la cantidad de elementos en el mismo.
*
* * */
int decodificar_contenido_de_mensaje( char** contenido_crudo, int* contenido_decodificado, int cantidad_de_elementos);

#endif