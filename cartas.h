#ifndef CARTAS_H
#define CARTAS_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//Definicion de los colores usados para imprimir las cartas
#define CODIGO_ANSI_COLOR_ROJO "\x1b[31m"
#define CODIGO_ANSI_COLOR_VERDE "\x1b[32m"
#define CODIGO_ANSI_COLOR_RESET "\x1b[0m"

//Definicion de los simbolos de cada palo
#define CODIGO_PALO_PICA CODIGO_ANSI_COLOR_VERDE"\xE2\x99\xA0"CODIGO_ANSI_COLOR_RESET
#define CODIGO_PALO_TREBOL CODIGO_ANSI_COLOR_VERDE"\xE2\x99\xA3"CODIGO_ANSI_COLOR_RESET
#define CODIGO_PALO_DIAMANTE CODIGO_ANSI_COLOR_ROJO"\xE2\x99\xA6"CODIGO_ANSI_COLOR_RESET
#define CODIGO_PALO_CORAZON CODIGO_ANSI_COLOR_ROJO"\xE2\x99\xA5"CODIGO_ANSI_COLOR_RESET

//Longitudes de cadenas de caracteres de uso interno
#define LONGITUD_CODIGO_PALO 12
#define LONGITUD_VALOR_CARTA 2
#define TAMANIO_DE_MAZO 52

/**
 * Esta funcion devuelve el nombre de la carta basado en su posicion en un mazo ordenado por numero y palo.
 * */
char* obtener_nombre_de_carta(int numero_carta);


/**
 * Obtiene el puntaje correspondiente a cada carta para calcular el valor de la mano de cada jugador
 * */
int obtener_puntos_de_carta(int numero_carta);

#endif
