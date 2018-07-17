#define _XOPEN_SOURCE

#include "servidor.h" 

/**
 * Se utilizan las siguientes variables globales para simplificar las firmas de las funciones internas del servidor.
 * Mejora la legibilidad.
 * */
int cantidad_de_jugadores;
int puntajes[CANTIDAD_MAXIMA_DE_JUGADORES];
jugador jugadores[CANTIDAD_MAXIMA_DE_JUGADORES];
bool partida_esta_en_progreso;
bool tiempo_de_espera_esta_cumplido;
bool servidor_esta_en_ejecucion;
bool turno_esta_terminado;
bool hay_jugadores_sin_cartas;
bool ronda_esta_terminada;

/**
 * El proposito de este arreglo de punteros a funciones es evitar tener muchos if-else o switch en la funcion main.
 * Los elementos con 0 son correspondientes a mensajes que solo son recibidos por el cliente.
 * Mejora la legibilidad.
 * */
fct_ptr funciones[] = { agregar_jugador, rechazar_conexion, agregar_apodo_a_jugador, 0, 0, 0, recibir_carta, 0, terminar_ronda, 0, actualizar_puntaje };

int main(int cantidad_de_argumentos, char** argumentos) {
	
	/**
	 * Declaracion de variables locales al main
	 * */
	int socket_servidor, maximo_descriptor, respuesta_del_select, i, maxima_cantidad_de_rondas, numero_de_ronda_actual = 0;
	struct sockaddr_in direccion_servidor, direccion_cliente;
	struct timeval timeout = {0, TIMEOUT_EN_MILISEGUNDOS};
	struct sigaction alarma, interrupcion;
	fd_set descriptor_de_archivos;
	char mensaje_vacio[TAMANIO_MENSAJE];
	char* mensaje;

	/**
	 * Parseo de los argumentos
	 * */
	if (cantidad_de_argumentos != 2) {
		fprintf(stderr, "Es necesario proveer el numero maximo de rondas como un argumento del programa.\n");
		return EXIT_FAILURE;
	}

	maxima_cantidad_de_rondas = atoi(argumentos[1]);

	/**Inicializacion del proveedor de numeros aleatorios
	 * */
	srand(time(NULL));

	/**
	 * Blanqueo de los sockets para evitar datos sucios
	 * */
	for (i = 0; i < CANTIDAD_MAXIMA_DE_JUGADORES; i++) {
		jugadores[i].socket = 0;
	}

	/**
	 * Se establecen los valores por defecto de los principales parametros del juego.
	 * */
	cantidad_de_jugadores = 0;
	partida_esta_en_progreso = FALSE;
	tiempo_de_espera_esta_cumplido = FALSE;
	servidor_esta_en_ejecucion = TRUE;

	/**
	 * Se establecen los gestores para la alarma para dejar de esperar jugadores y otras interrupciones.
	 * */
	memset(&alarma, 0, sizeof(alarma));
	memset(&interrupcion, 0, sizeof(interrupcion));
	alarma.sa_handler = &gestor_de_alarmas;
	interrupcion.sa_handler = &gestor_de_interrupciones;
	sigaction(SIGALRM, &alarma, NULL);
	sigaction(SIGINT, &interrupcion, NULL);

	inicializar_servidor(&socket_servidor, &direccion_servidor);

	while (servidor_esta_en_ejecucion) {
		FD_ZERO(&descriptor_de_archivos);
		FD_SET(socket_servidor, &descriptor_de_archivos);
		maximo_descriptor = socket_servidor + 1;
		int i;

		for (i = 0; i < CANTIDAD_MAXIMA_DE_JUGADORES; i++) {
			if (jugadores[i].socket > 0) {
				FD_SET(jugadores[i].socket, &descriptor_de_archivos);
			}

			if (jugadores[i].socket >= maximo_descriptor) {
				maximo_descriptor = jugadores[i].socket+1;
			}
		}
		if ((respuesta_del_select = select(maximo_descriptor, &descriptor_de_archivos, NULL, NULL, &timeout)) < 0) {
			//Lo unico que interrumpe el select es nuestra alarma para determinar que ya no esperamos mas
			if (errno != EINTR) {
				apagar_servidor();
				return EXIT_FAILURE;
			}
		}

		//Si la respuesta fuera 0, es porque esperamos jugadores sin que se conecte ninguno
		if (respuesta_del_select > 0) {
			if (FD_ISSET(socket_servidor, &descriptor_de_archivos)) {
				agregar_cliente(socket_servidor, &direccion_cliente);
			}

			for (i = 0; i < cantidad_de_jugadores; i++) {
				if (FD_ISSET(jugadores[i].socket, &descriptor_de_archivos)) {
					mensaje = mensaje_vacio;

					if (recibir_mensaje(mensaje, jugadores[i].socket)) {
						int msg_code = extraer_codigo_de_mensaje(&mensaje);
						funciones[msg_code] (jugadores[i].socket, &mensaje);
					} else {
						remover_jugador(jugadores, i, FALSE);
					}
				}
			}
		}

		if (partida_esta_en_progreso) {
			if (turno_esta_terminado && !hay_jugadores_sin_cartas) {
				//El turno se termino y ningun jugador dijo haberse quedado sin cartas
				turno_esta_terminado = FALSE;
				//Le pedimos a los jugadores que jueguen
				broadcast_liviano(JUEGUE, jugadores, cantidad_de_jugadores);
			}
			
			if (ronda_esta_terminada) {
				numero_de_ronda_actual++;

				if (numero_de_ronda_actual == maxima_cantidad_de_rondas) {
					terminar_partida();
					numero_de_ronda_actual = 0;
					printf("PARTIDA TERMINADA!\n");
				} else {
					comenzar_ronda();
				}
			}
		}
	}

	/**
	 * Se cierra el socket del servidor previo a terminar la ejecucion del servidor.
	 * */
	cerrar_socket(socket_servidor);
	return EXIT_SUCCESS;
}

/**
 * Esta funcion maneja el evento que surge luego de cumplido el tiempo de espera para comenzar la partida
 * */
void gestor_de_alarmas(int numero_senial) {
	if (numero_senial == SIGALRM) {
		
		//Si no juntamos al menos dos jugadores, no se puede jugar
		if (cantidad_de_jugadores < 2) {
			vaciar_sala_de_espera();
		} else {
			comenzar_partida();
		}

		tiempo_de_espera_esta_cumplido = TRUE;
	}
}

int encontrar_posicion_jugador(jugador* jugadores, int socket) {
	int i;

	for (i = 0; i < CANTIDAD_MAXIMA_DE_JUGADORES; i++) {
		if (jugadores[i].socket == socket) {
			return i;
		}
	}
	return -1;
}


void gestor_de_interrupciones(int signum) {
	if (signum == SIGINT) {
		apagar_servidor();
	}
}

void inicializar_servidor(int *socket_servidor, struct sockaddr_in *direccion_servidor) {

	if ((*socket_servidor = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("ERROR: Fallo en la inicializacion del socket.");
		exit(EXIT_FAILURE);
	}

	memset(direccion_servidor, 1, sizeof(struct sockaddr_in));
	direccion_servidor->sin_family = AF_INET;
	direccion_servidor->sin_port = htons(PORT);
	direccion_servidor->sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(*socket_servidor, (struct sockaddr *)direccion_servidor, sizeof(*direccion_servidor)) == -1) {
		perror("ERROR: Fallo en la vinculacion del socket.");
		exit(EXIT_FAILURE);
	}

	if (listen(*socket_servidor, BACKLOG)) {
		perror("ERROR: Fallo al intentar escuchar el socket.");
		exit(EXIT_FAILURE);
	}
}

void agregar_cliente(int socket_servidor, struct sockaddr_in *direccion_cliente) {
	int nuevo_socket_de_cliente;
	int longitud_direccion_cliente = sizeof(struct sockaddr_in);

	if ((nuevo_socket_de_cliente = accept(socket_servidor, (struct sockaddr *)direccion_cliente, (socklen_t*) &longitud_direccion_cliente)) < 0) {
		perror("ERROR: No se pudo conectar.");
		exit(EXIT_FAILURE);
	} else {
		//Aceptamos o rechazamos la conexion, segun haya lugar en la partida o no
		if (partida_esta_en_progreso || cantidad_de_jugadores == CANTIDAD_MAXIMA_DE_JUGADORES) {
			rechazar_conexion(nuevo_socket_de_cliente);
		} else {
			agregar_jugador(nuevo_socket_de_cliente);
		}
	}
}

void agregar_jugador(int socket) {
	jugadores[cantidad_de_jugadores].carta_jugada = -1;
	jugadores[cantidad_de_jugadores].se_quedo_sin_cartas = FALSE;
	jugadores[cantidad_de_jugadores++].socket = socket;
	enviar_mensaje_numerico(ESPERANDO_JUGADORES, CUENTA_REGRESIVA, socket);

	if (cantidad_de_jugadores == 1) {
		//Primer jugador, ponemos la alarma de 30 segundos
		alarm(CUENTA_REGRESIVA);
	}
}

void terminar_partida() {
	printf("CANTIDAD DE JUGADORES: %d\n", cantidad_de_jugadores);
	int i, posicion_ganador, puntaje_maximo = 0;

	for (i = 0; i < cantidad_de_jugadores; i++) {
		if (puntajes[i] >= puntaje_maximo) {
			puntaje_maximo = puntajes[i];
			posicion_ganador = i;
		}
	}

	printf("EL GANADOR ES : %s CON %d PUNTOS.s\n", jugadores[posicion_ganador].apodo, puntajes[posicion_ganador]);
	enviar_mensaje_sin_cuerpo(GANADOR, jugadores[posicion_ganador].socket);
	vaciar_sala_de_espera();
}

void remover_jugador(jugador* jugadores, int posicion_a_remover, bool debe_cerrar_socket) {
	if (debe_cerrar_socket) {
		cerrar_socket(jugadores[posicion_a_remover].socket);
	}

	memcpy(jugadores[posicion_a_remover].apodo, "\0", TAMANIO_DE_NOMBRE);
	jugadores[posicion_a_remover].socket = 0;
	puntajes[posicion_a_remover] = 0;
	cantidad_de_jugadores--;
	int j;

	for (j = posicion_a_remover + 1; j <= cantidad_de_jugadores; j++) { 
		jugadores[j-1].socket = jugadores[j].socket;
		sprintf(jugadores[j - 1].apodo, "%s", jugadores[j].apodo);
	}

	if (partida_esta_en_progreso && cantidad_de_jugadores == 1) {
		terminar_partida();
	}
}

void rechazar_conexion(int socket) {
	enviar_mensaje_sin_cuerpo(CONEXION_RECHAZADA, socket);
}

void agregar_apodo_a_jugador(int socket, char** mensaje) {
	char apodo[TAMANIO_DE_NOMBRE];
	extraer_apodo_de_mensaje(mensaje, apodo);
	int index = encontrar_posicion_jugador(jugadores, socket);
	sprintf(jugadores[index].apodo, "%s", apodo);
}

void repartir_cartas() {
	int cantidad_de_cartas_por_jugador = TAMANIO_DE_MAZO / cantidad_de_jugadores;
	int cartas_repartidas[cantidad_de_cartas_por_jugador * cantidad_de_jugadores];
	int cantidad_de_cartas_repartidas = 0;
	int i;
	
	for (i = 0; i < cantidad_de_jugadores; i++) {
		int carta;
		int tamanio_mensaje = 0;
		char mensaje[3 * cantidad_de_cartas_por_jugador]; //2 caracteres por carta mas un espacio
		mensaje[0] = '\0';

		printf("CARTAS REPARTIDAS a '%s':", jugadores[i].apodo);

		for (carta = 0; carta < cantidad_de_cartas_por_jugador; carta++) {
			int carta_aleatoria;

			do {
				carta_aleatoria = obtener_numero_aleatorio(TAMANIO_DE_MAZO);
			} while (arreglo_contiene(cartas_repartidas, carta_aleatoria, cantidad_de_cartas_repartidas));
			
			printf(" %s |", obtener_nombre_de_carta(carta_aleatoria));

			//Agregamos la carta al mazo correspondiente
			tamanio_mensaje += sprintf(mensaje + tamanio_mensaje, "%d ", carta_aleatoria);
			cartas_repartidas[cantidad_de_cartas_repartidas++] = carta_aleatoria;
		}

		//Enviamos lo que le toco al jugador
		enviar_mensaje(NUEVA_MANO, mensaje, jugadores[i].socket);
		printf("\n");
	}
}

void vaciar_sala_de_espera() {
	broadcast_liviano(DESCONEXION, jugadores, cantidad_de_jugadores);
	partida_esta_en_progreso = FALSE;

	while (jugadores[0].socket > 0) {
			remover_jugador(jugadores, 0, FALSE);
	}
}

bool recibir_mensaje(char* mensaje, int descriptor_de_archivo) {
	int bytes_recibidos;

	if ((bytes_recibidos = recv(descriptor_de_archivo, mensaje, TAMANIO_MENSAJE, 0)) <= 0) {
		if (bytes_recibidos == 0) {
			printf("SE DESCONECTO EL CLIENTE.\n");
		}
		else {
			perror("ERROR: No se pudo recibir el mensaje.");
		}

		return FALSE;
	}

	return TRUE;
}

void comenzar_partida() {
	comenzar_ronda();
	partida_esta_en_progreso = TRUE;
}

void comenzar_ronda() {
	repartir_cartas();
	turno_esta_terminado = TRUE;
	ronda_esta_terminada = FALSE;
	hay_jugadores_sin_cartas = FALSE;
}

void cerrar_socket(int socket) {
	printf("INFO: Cerrando socket #%d\n", socket);

	if (close(socket) < 0) {
		perror("ERROR: Fallo al intentar cerrar un socket.");
		exit(EXIT_FAILURE);
	}

	printf("INFO: Se pudo cerrar exitosamente el socket #%d\n", socket);
}

void apagar_servidor() {
	printf("INFO: Servidor apagandose.\n");
	vaciar_sala_de_espera();
	servidor_esta_en_ejecucion = FALSE;
}

void recibir_carta(int socket, char** mensaje) {
	static int cantidad_de_cartas_recibidas = 0;
	static int tamanio_mensaje = 0;
	static char cartas[CANTIDAD_MAXIMA_DE_JUGADORES * 3];
	int posicion_jugador = encontrar_posicion_jugador(jugadores, socket);
	int card;

	decodificar_contenido_de_mensaje(mensaje, &card, 1);
	jugadores[posicion_jugador].carta_jugada = card;
	cantidad_de_cartas_recibidas++;
	tamanio_mensaje += sprintf(cartas + tamanio_mensaje, "%d ", card);

	if (cantidad_de_cartas_recibidas == cantidad_de_jugadores) {
		int posicion_jugador_con_carta_mas_alta, carta_mas_alta = 0, i, cantidad_de_jugadores_sin_cartas = 0, posicion_jugador_sin_cartas;

		for (i = 0; i < cantidad_de_jugadores; i++) {
			if (jugadores[i].carta_jugada >= carta_mas_alta) {
				carta_mas_alta = jugadores[i].carta_jugada;
				posicion_jugador_con_carta_mas_alta = i;
			}

			if (jugadores[i].se_quedo_sin_cartas) {
				cantidad_de_jugadores_sin_cartas++;
				posicion_jugador_sin_cartas = i;
			}

			jugadores[i].carta_jugada = -1;
			jugadores[i].se_quedo_sin_cartas = FALSE;
		}

		enviar_mensaje(CARTAS_GANADAS, cartas, jugadores[posicion_jugador_con_carta_mas_alta].socket);
		memset(cartas, 0, cantidad_de_jugadores * sizeof(int));
		cantidad_de_cartas_recibidas = 0;
		carta_mas_alta = -1;
		tamanio_mensaje = 0;
		turno_esta_terminado = TRUE;

		if (cantidad_de_jugadores_sin_cartas == 1 && posicion_jugador_sin_cartas == posicion_jugador_con_carta_mas_alta) {
			//Si el jugador sin cartas gana el turno, la ronda sigue
			printf("INFO: El jugador %s se salvo justo!\n", jugadores[posicion_jugador_con_carta_mas_alta].apodo);
			hay_jugadores_sin_cartas = FALSE;
		}

		if (hay_jugadores_sin_cartas) {
			broadcast_liviano(RONDA_TERMINADA, jugadores, cantidad_de_jugadores);
		}
	}
}

void terminar_ronda(int socket, char** mensaje) {
	if (!hay_jugadores_sin_cartas) {
		printf("INFO: Termino la ronda.\n");
		hay_jugadores_sin_cartas = TRUE;
	}

	jugadores[encontrar_posicion_jugador(jugadores, socket)].se_quedo_sin_cartas = TRUE;
	printf("INFO: El mensaje es %s.\n", *mensaje);
}

void actualizar_puntaje(int socket, char** mensaje) {
	static int count = 0;
	int puntaje_jugador;
	int i;

	decodificar_contenido_de_mensaje(mensaje, &puntaje_jugador, 1);

	for (i = 0; i < cantidad_de_jugadores; i++) {
		if (jugadores[i].socket == socket) {
			puntajes[i] += puntaje_jugador;
		}
	}

	count++;

	if (count == cantidad_de_jugadores) {
		count = 0;
		ronda_esta_terminada = TRUE;

		for (i = 0; i < cantidad_de_jugadores; i++) {
			if (jugadores[i].socket > 0) {
				printf("EL JUGADOR %s tiene %d puntos.\n", jugadores[i].apodo, puntajes[i]);
			}
		}

		char buffer[10];
		printf("INFO: Presionar cualquier tecla para comenzar proxima ronda.\n");
		fgets(buffer, 10, stdin);
	}
}

