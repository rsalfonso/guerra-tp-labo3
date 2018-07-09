/*
 * =====================================================================================
 *
 *       Filename:  client.c
 *
 *    Description:  Fichier gérant le client du projet
 *
 *        Version:  1.0
 *        Created:  05/04/2016 03:54:34 PM
 *       Revision:  1
 *       Compiler:  gcc
 *
 *         Author:  DIMOV Theodor, DRAGOMIR Philippe
 *   Organization:  IPL-Student
 *
 * =====================================================================================
 */
#include "client.h"

int client_socket;
int hand[TAMANIO_DE_MAZO];
int stash[TAMANIO_DE_MAZO];
int cards_in_hand;
int cards_in_stash;

void disconnect(bool should_inform) {
	printf("Hasta luego..\n");
	if (should_inform) {
		enviar_mensaje_sin_cuerpo(DESCONEXION, client_socket);
		close(client_socket);
	}
	exit(EXIT_SUCCESS);
}
void interrupt_handler(int signum) {
	if (signum == SIGINT) {
		//Ctr+C
		disconnect(TRUE);
	}
}

void refill() {
	memcpy(hand, stash, cards_in_stash * sizeof(int));
	memset(stash, -1, cards_in_stash * sizeof(int));
	cards_in_hand = cards_in_stash;
	cards_in_stash = 0;
}

void clear_cards() {
	memset(hand, -1, TAMANIO_DE_MAZO * sizeof(int));
	memset(stash, -1, TAMANIO_DE_MAZO * sizeof(int));
	cards_in_hand = 0;
	cards_in_stash = 0;
}

void print_cards() {
	int i;
	printf("mano actual : ");
	for (i = 0; i < cards_in_hand; i++) {
		printf("%d: %-8s ", i, obtener_nombre_de_carta(hand[i]));
		if(i+1 < cards_in_hand) {
			printf("|");
		}
	}
	printf("\n");
	printf("cartas en la pila : ");
	for (i = 0; i < cards_in_stash; i++) {
		printf("%d: %-8s ", i, obtener_nombre_de_carta(stash[i]));
		if(i+1 < cards_in_stash ){
			printf("|");
		}
	}
	printf("\n");
}

int calculate_score() {
	printf("CALCULANDO PUNTAJE!\n");
	print_cards();
	int score = 0;
	int i;
	for (i = 0; i < cards_in_hand; i++) {
		score += obtener_puntos_de_carta(hand[i]);
	}
	for (i = 0; i < cards_in_stash; i++) {
		score += obtener_puntos_de_carta(stash[i]);
	}
	return score;
}

void receive_message(int client_socket,char** name) {
	char msgFromServer[TAMANIO_MENSAJE];
	char* msg = msgFromServer;

	int bytesReceived;
	if ((bytesReceived=recv(client_socket,msgFromServer,TAMANIO_MENSAJE,0))==-1) {
		perror("Receive");
		exit(EXIT_FAILURE);
	}
	msgFromServer[bytesReceived]='\0';
	fflush(stdout);
	int msg_code = extraer_codigo_de_mensaje(&msg);
	if(msg_code == ESPERANDO_JUGADORES) {
		enviar_mensaje(APODO, *name, client_socket);
	} else if(msg_code == CONEXION_RECHAZADA) {
		printf("Conexion rechazada por el servidor. Por favor intentelo mas tarde.\n");
		close(client_socket);
		exit(-1);
	} else if(msg_code == DESCONEXION) {
		printf("Desconectado por el servidor.\n");
		printf("Fin del juego!\n\n");
		close(client_socket);
		exit(-1);
	} else if(msg_code == RONDA_TERMINADA) {
		printf("La ronda actual termino.\n");
		int score = calculate_score();
		enviar_mensaje_numerico(PUNTAJE, score, client_socket);
		printf("Su puntaje : %d .. \n", score);
	} else if(msg_code == NUEVA_MANO) {
		clear_cards();
		printf("Comenzando la ronda. ");
		cards_in_hand = decodificar_contenido_de_mensaje(&msg, hand, TAMANIO_DE_MAZO / 2);
		printf("Estas son sus cartas : \n");
	} else if(msg_code == JUEGUE) {
		print_cards();
		int choice = -1;
		if (cards_in_hand + cards_in_stash == 1) {
			printf("Usted esta jugando su ultima carta\n");
			//the player is about to play his last card, the round is over
			enviar_mensaje_sin_cuerpo(ULTIMA_CARTA, client_socket);
		}
		if (cards_in_hand == 0) {
			//the player hasn't got any cards in his hand, his stash becomes his hand
			refill();
			printf("Sin cartas, tomando cartas de la pila\n");
			print_cards();
		}
		int times = 0;
		do {
			if(times > 0) {
				printf("Por favor, juegue una de sus cartas.\n");
				print_cards();
			}
			printf("Que carta le gustaria jugar ?\n");
			if (scanf("%d", &choice) == EOF) {
				//Ctr+D
				disconnect(FALSE);
			}
			times++;
		} while(choice < 0 || choice > cards_in_hand);
		enviar_mensaje_numerico(JUGADA, hand[choice], client_socket);
		int i;
		for (i = choice; i < cards_in_hand-1; i++) {
			hand[i] = hand[i+1];
		}
		cards_in_hand--;
		printf("Su nueva mano es: ");
		print_cards();
		printf("\n");
	} else if (msg_code == CARTAS_GANADAS){
		int* stash_ptr = stash + cards_in_stash;
		int size = decodificar_contenido_de_mensaje(&msg, stash_ptr, CANTIDAD_MAXIMA_DE_JUGADORES);
		cards_in_stash += size;
		print_cards();
		printf("Ha ganado el turno!\n");
	} else if (msg_code == GANADOR) {
		printf("Felicidades, ha ganado el juego!\n");
	} else {
		printf("Mensaje no soportado !");
	}
}

void create_nickname(char* name) {
	char* request="Ingrese su nombre: ";
	printf("%s", request);
	scanf("%s", name);
	fflush(stdin);
	fflush(stdout);
}

void connectToServer(int *client_socket,char* server_ip,struct hostent *host,struct sockaddr_in *server_address){
	if ((host=gethostbyname(server_ip)) == NULL) {
		perror("Client: gethostbyname failed");
		exit(EXIT_FAILURE);
	}

	if ((*client_socket = socket(AF_INET,SOCK_STREAM, 0)) == -1) {
		perror("Client: socket");
		exit(EXIT_FAILURE);
	}

	server_address->sin_family = AF_INET;
	server_address->sin_port = htons(PUERTO_SERVIDOR);
	server_address->sin_addr = *((struct in_addr*)host->h_addr);
	memset(&(server_address->sin_zero), '\0', 8);

	if (connect(*client_socket, (struct sockaddr *)server_address,sizeof(struct sockaddr)) == -1) {
		perror("Client: connect");
		exit(EXIT_FAILURE);
	}
}

bool fdp_is_valid(int fdp) {
	return fcntl(fdp, F_GETFD) != -1 || errno != EBADF;
}

int main(int argc, char *argv[]) {

	if (argc != 2) {
		fprintf(stderr, "Debe ingresar la direccion IP del servidor.\n");
		return EXIT_FAILURE;
	}
	struct sockaddr_in server_address;// adresse du server
	struct hostent host;
	clear_cards();
	char name[TAMANIO_DE_NOMBRE];
	char* name_ptr = name;

	struct sigaction interrupt;
	memset(&interrupt, 0, sizeof(interrupt));
	interrupt.sa_handler = &interrupt_handler;
	sigaction(SIGINT, &interrupt, NULL);

	create_nickname(name);
	connectToServer(&client_socket, argv[1], &host, &server_address);
	while (1) {
		if(!fdp_is_valid(client_socket)) {
			printf("Connection error.\n");
			break;
		}
		receive_message(client_socket, &name_ptr);
	}
	close(client_socket);
	return EXIT_SUCCESS;
}
