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
#include "config.h"
#include "common_utils.h"
#include "cartas.h"

#define PORT PORT_DIMOV
#endif

void interrupt_handler(int);
void disconnect(bool);
void refill();
void clear_cards();
void print_cards();
int calculate_score();
void receive_message(int clientSocket,char** name);
void createNickname(char *name);
void connectToServer(int *clientSocket,char* serverIP,struct hostent *he,struct sockaddr_in *serverAddress);
bool fdp_is_valid(int fdp);