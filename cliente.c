#include "cliente.h"

int socket_cliente;
int mano[TAMANIO_DE_MAZO];
int pila[TAMANIO_DE_MAZO];
int cartas_en_mano;
int cartas_en_pila;

void desconectar(bool debe_informar) {
	printf("Hasta luego..\n");
	if (debe_informar) {
		enviar_mensaje_sin_cuerpo(DESCONEXION, socket_cliente);
		close(socket_cliente);
	}
	exit(EXIT_SUCCESS);
}
void manejar_interrupcion(int signum) {
	if (signum == SIGINT) {
		//Ctr+C
		desconectar(TRUE);
	}
}

void reponer() {
	memcpy(mano, pila, cartas_en_pila * sizeof(int));
	memset(pila, -1, cartas_en_pila * sizeof(int));
	cartas_en_mano = cartas_en_pila;
	cartas_en_pila = 0;
}

void limpiar_cartas() {
	memset(mano, -1, TAMANIO_DE_MAZO * sizeof(int));
	memset(pila, -1, TAMANIO_DE_MAZO * sizeof(int));
	cartas_en_mano = 0;
	cartas_en_pila = 0;
}

void imprimir_cartas() {
	int i;
	printf("mano actual : ");
	for (i = 0; i < cartas_en_mano; i++) {
		printf("%d: %-8s ", i, obtener_nombre_de_carta(mano[i]));
		if (i+1 < cartas_en_mano) {
			printf("|");
		}
	}
	printf("\n");
	printf("cartas en la pila : ");
	for (i = 0; i < cartas_en_pila; i++) {
		printf("%d: %-8s ", i, obtener_nombre_de_carta(pila[i]));
		if (i+1 < cartas_en_pila ){
			printf("|");
		}
	}
	printf("\n");
}

int calcular_puntaje() {
	printf("CALCULANDO PUNTAJE!\n");
	imprimir_cartas();
	int puntaje = 0;
	int i;
	for (i = 0; i < cartas_en_mano; i++) {
		puntaje += obtener_puntos_de_carta(mano[i]);
	}
	for (i = 0; i < cartas_en_pila; i++) {
		puntaje += obtener_puntos_de_carta(pila[i]);
	}
	return puntaje;
}

void recibir_mensaje(int socket_cliente,char** nombre) {
	char mensaje_servidor[TAMANIO_MENSAJE];
	char* mensaje = mensaje_servidor;

	int bytes_recibidos;

	if ((bytes_recibidos = recv(socket_cliente,mensaje_servidor,TAMANIO_MENSAJE,0)) == -1) {
		perror("Receive");
		exit(EXIT_FAILURE);
	}

	mensaje_servidor[bytes_recibidos] = '\0';
	fflush(stdout);
	int codigo_de_mensaje = extraer_codigo_de_mensaje(&mensaje);

	if (codigo_de_mensaje == ESPERANDO_JUGADORES) {
		enviar_mensaje(APODO, *nombre, socket_cliente);
	} else if (codigo_de_mensaje == CONEXION_RECHAZADA) {
		printf("Conexion rechazada por el servidor. Por favor intentelo mas tarde.\n");
		close(socket_cliente);
		exit(-1);
	} else if (codigo_de_mensaje == DESCONEXION) {
		printf("Desconectado por el servidor.\n");
		printf("Fin del juego!\n\n");
		close(socket_cliente);
		exit(-1);
	} else if (codigo_de_mensaje == RONDA_TERMINADA) {
		printf("La ronda actual termino.\n");
		int puntaje = calcular_puntaje();
		enviar_mensaje_numerico(PUNTAJE, puntaje, socket_cliente);
		printf("Su puntaje : %d .. \n", puntaje);
	} else if (codigo_de_mensaje == NUEVA_MANO) {
		limpiar_cartas();
		printf("Comenzando la ronda. ");
		cartas_en_mano = decodificar_contenido_de_mensaje(&mensaje, mano, TAMANIO_DE_MAZO / 2);
		printf("Estas son sus cartas : \n");
	} else if (codigo_de_mensaje == JUEGUE) {
		imprimir_cartas();
		int eleccion = -1;
		if (cartas_en_mano + cartas_en_pila == 1) {
			printf("Usted esta jugando su ultima carta\n");
			//El jugador esta a punto de jugar su ultima carta, la ronda esta terminada
			enviar_mensaje_sin_cuerpo(ULTIMA_CARTA, socket_cliente);
		}
		if (cartas_en_mano == 0) {
			//El jugador no tiene ninguna carta en la mano, la pila se convierte en su mano.
			reponer();
			printf("Sin cartas, tomando cartas de la pila\n");
			imprimir_cartas();
		}

		int intentos = 0;
		do {
			if (intentos > 0) {
				printf("Por favor, juegue una de sus cartas.\n");
				imprimir_cartas();
			}
			printf("Que carta le gustaria jugar ?\n");
			if (scanf("%d", &eleccion) == EOF) {
				//Ctr+D
				desconectar(FALSE);
			}
			intentos++;
		} while(eleccion < 0 || eleccion > cartas_en_mano);

		enviar_mensaje_numerico(JUGADA, mano[eleccion], socket_cliente);
		int i;
		
		for (i = eleccion; i < cartas_en_mano-1; i++) {
			mano[i] = mano[i+1];
		}
		
		cartas_en_mano--;
		printf("Su nueva mano es: ");
		imprimir_cartas();
		printf("\n");
	} else if (codigo_de_mensaje == CARTAS_GANADAS){
		int* puntero_pila = pila + cartas_en_pila;
		int size = decodificar_contenido_de_mensaje(&mensaje, puntero_pila, CANTIDAD_MAXIMA_DE_JUGADORES);
		cartas_en_pila += size;
		imprimir_cartas();
		printf("Ha ganado el turno!\n");
	} else if (codigo_de_mensaje == GANADOR) {
		printf("Felicidades, ha ganado el juego!\n");
	} else {
		printf("Mensaje no soportado !");
	}
}

void crear_sobrenombre(char* nombre) {
	char* mensaje = "Ingrese su nombre: ";
	printf("%s", mensaje);
	scanf("%s", nombre);
	fflush(stdin);
	fflush(stdout);
}

void conectar_con_servidor(int *socket_cliente,char* ip_servidor,struct hostent *host,struct sockaddr_in *direccion_servidor){
	if ((host = gethostbyname(ip_servidor)) == NULL) {
		perror("Cliente: gethostbyname fallo");
		exit(EXIT_FAILURE);
	}

	if ((*socket_cliente = socket(AF_INET,SOCK_STREAM, 0)) == -1) {
		perror("Cliente: socket");
		exit(EXIT_FAILURE);
	}

	direccion_servidor->sin_family = AF_INET;
	direccion_servidor->sin_port = htons(PUERTO_SERVIDOR);
	direccion_servidor->sin_addr = *((struct in_addr*)host->h_addr);
	memset(&(direccion_servidor->sin_zero), '\0', 8);

	if (connect(*socket_cliente, (struct sockaddr *)direccion_servidor,sizeof(struct sockaddr)) == -1) {
		perror("Cliente: connect");
		exit(EXIT_FAILURE);
	}
}

bool validar_fdp(int fdp) {
	return fcntl(fdp, F_GETFD) != -1 || errno != EBADF;
}

int main(int argc, char *argv[]) {

	if (argc != 2) {
		fprintf(stderr, "Debe ingresar la direccion IP del servidor.\n");
		return EXIT_FAILURE;
	}
	struct sockaddr_in direccion_servidor;// adresse du server
	struct hostent host;
	limpiar_cartas();
	char nombre[TAMANIO_DE_NOMBRE];
	char* puntero_nombre = nombre;

	struct sigaction interrupcion;
	memset(&interrupcion, 0, sizeof(interrupcion));
	interrupcion.sa_handler = &manejar_interrupcion;
	sigaction(SIGINT, &interrupcion, NULL);

	crear_sobrenombre(nombre);
	conectar_con_servidor(&socket_cliente, argv[1], &host, &direccion_servidor);
	while (1) {
		if (!validar_fdp(socket_cliente)) {
			printf("Error de conexion.\n");
			break;
		}
		recibir_mensaje(socket_cliente, &puntero_nombre);
	}
	close(socket_cliente);
	return EXIT_SUCCESS;
}
