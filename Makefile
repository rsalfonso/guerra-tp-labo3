TARGET= server client

CC=gcc
CFLAGS=-Wall -Wextra

normal: $(TARGET)

server: server.o mensajeria.o server_utils.o cartas.o
	$(CC) $(CFLAGS) server.o mensajeria.o server_utils.o cartas.o -o server

client: client.o mensajeria.o cartas.o
	$(CC) $(CFLAGS) client.o mensajeria.o cartas.o -o client

server.o: server.c mensajeria.h server_utils.h cartas.h
	$(CC) $(CFLAGS) -c server.c

client.o: client.c mensajeria.h cartas.h
	$(CC) $(CFLAGS) -c client.c

mensajeria.o:	mensajeria.h mensajeria.c
	$(CC) $(CFLAGS) -c mensajeria.c

server_utils.o: server_utils.h server_utils.c
	$(CC) $(CFLAGS) -c server_utils.c

cartas.o: cartas.h cartas.c
	$(CC) $(CFLAGS) -c cartas.c

clean:
	$(RM) $(TARGET)
