#ifndef CLIENTH_H
#define CLIENTH_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>
#include <signal.h>
#include "configuracion.h"
#include "mensajeria.h"
#include "cartas.h"

#define PORT PUERTO_SERVIDOR
#endif

void interrupt_handler(int);
void desconectar(bool);
void reponer();
void limpiar_cartas();
void imprimir_cartas();
int calcular_puntaje();
void recibir_mensaje(int clientSocket,char** name);
void crear_sobrenombre(char *name);
void conectar_con_servidor(int *clientSocket,char* serverIP,struct hostent *he,struct sockaddr_in *serverAddress);
bool validar_fdp(int fdp);