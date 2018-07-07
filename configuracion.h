#ifndef _CONFIGURACION_H
#define _CONFIGURACION_H

typedef int bool;
#define TRUE 1
#define FALSE 0

//Codigos de mensaje desde el servidor hacia los clientes
#define ESPERANDO_JUGADORES 0
#define CONEXION_RECHAZADA 1
#define NUEVA_MANO 4
#define JUEGUE 5
#define CARTAS_GANADAS 7
#define RONDA_TERMINADA	9
#define GANADOR	12

//Codigos de mensaje desde los clientes hacia el servidor
#define APODO 2
#define JUGADA 6
#define ULTIMA_CARTA 8
#define PUNTAJE 10

//Codigos de mensaje bidireccionales
#define DESCONEXION	3

//Puertos usados
#define PUERTO_SERVIDOR 	17626

#define CANTIDAD_MAXIMA_DE_JUGADORES 4
#define TAMANIO_DE_NOMBRE 20
#define TAMANIO_MENSAJE 82

#endif
