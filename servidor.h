#ifndef SERVIDOR_H
#define SERVIDOR_H

#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include "mensajeria.h"
#include "utilidades.h"
#include "cartas.h"

//#define MIN_PLAYERS 2
#define PORT PUERTO_SERVIDOR
//#define BUFFER_SIZE 1024
#define BACKLOG 5
#define CUENTA_REGRESIVA 10
#define TIMEOUT_EN_MILISEGUNDOS 15000

typedef void (*fct_ptr)( );

void inicializar_servidor(int *socket_servidor, struct sockaddr_in *direccion_servidor);
void gestor_de_alarmas(int numero_senial);
void gestor_de_interrupciones(int signum);
void cerrar_socket(int socket);
void apagar_servidor();
void agregar_cliente(int socket_servidor, struct sockaddr_in *direccion_cliente);
void agregar_jugador(int socket);
void remover_jugador(jugador* jugadores, int posicion_a_remover, bool cerrar_socket);
void rechazar_conexion(int socket);
bool recibir_mensaje(char* mensaje, int descriptor_de_archivo);
void vaciar_sala_de_espera();
void agregar_apodo_a_jugador(int socket, char** mensaje);
void repartir_cartas();
void comenzar_partida();
void comenzar_ronda();
void recibir_carta(int socket, char** mensaje);
void terminar_ronda(int socket, char** mensaje);
void actualizar_puntaje(int socket, char** mensaje);
int encontrar_posicion_jugador(jugador* jugadores, int socket);
void terminar_partida();


#endif