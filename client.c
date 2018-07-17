#include "client.h"

int client_socket;
int hand[TAMANIO_DE_MAZO];
int stash[TAMANIO_DE_MAZO];
int cards_in_hand;
int cards_in_stash;

void disconnect(bool should_inform) {
	printf("Okay, okay..\n");
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
	printf("current hand : ");
	for (i = 0; i < cards_in_hand; i++) {
		printf("%d: %-8s ", i, obtener_nombre_de_carta(hand[i]));
		if(i+1 < cards_in_hand) {
			printf("|");
		}
	}
	printf("\n");
	printf("current stash : ");
	for (i = 0; i < cards_in_stash; i++) {
		printf("%d: %-8s ", i, obtener_nombre_de_carta(stash[i]));
		if(i+1 < cards_in_stash ){
			printf("|");
		}
	}
	printf("\n");
}

int calculate_score() {
	printf("CALCULATING PUNTAJE!\n");
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
		printf("Connection refused by the server. Please try again later.\n");
		close(client_socket);
		exit(-1);
	} else if(msg_code == DESCONEXION) {
		printf("Disconnected by the server.\n");
		printf("Game over!\n\n");
		close(client_socket);
		exit(-1);
	} else if(msg_code == RONDA_TERMINADA) {
		printf("The current round is over.\n");
		int score = calculate_score();
		enviar_mensaje_numerico(PUNTAJE, score, client_socket);
		printf("Your score : %d .. \n", score);
	} else if(msg_code == NUEVA_MANO) {
		clear_cards();
		printf("Round begins. ");
		cards_in_hand = decodificar_contenido_de_mensaje(&msg, hand, TAMANIO_DE_MAZO / 2);
		printf("These are your cards : \n");
	} else if(msg_code == JUEGUE) {
		print_cards();
		int choice = -1;
		if (cards_in_hand + cards_in_stash == 1) {
			printf("You are playing your last card\n");
			//the jugador is about to play his last card, the round is over
			enviar_mensaje_sin_cuerpo(ULTIMA_CARTA, client_socket);
		}
		if (cards_in_hand == 0) {
			//the jugador hasn't got any cards in his hand, his stash becomes his hand
			refill();
			printf("Out of cards, hand replenished from stash\n");
			print_cards();
		}
		int times = 0;
		do {
			if(times > 0) {
				printf("You are kindly requested to play one of your own cards\n");
				print_cards();
			}
			printf("Which card would you like to play ?\n");
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
		printf("Your new hand is: ");
		print_cards();
		printf("\n");
	} else if (msg_code == CARTAS_GANADAS){
		int* stash_ptr = stash + cards_in_stash;
		int size = decodificar_contenido_de_mensaje(&msg, stash_ptr, CANTIDAD_MAXIMA_DE_JUGADORES);
		cards_in_stash += size;
		print_cards();
		printf("You win the turn!\n");
	} else if (msg_code == GANADOR) {
		printf("Congratulations, you win the game!\n");
	} else {
		printf("Message not support yet !");
	}
}

void create_nickname(char* name) {
	char* request="Enter your apodo (20 characters max): ";
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
		fprintf(stderr, "You need to specify the server's ip address\n");
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
