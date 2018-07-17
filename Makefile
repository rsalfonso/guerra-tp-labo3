TARGET= servidor cliente

CC=gcc
CFLAGS=-Wall -Wextra

normal: $(TARGET)

servidor: servidor.o mensajeria.o utilidades.o cartas.o
	$(CC) $(CFLAGS) servidor.o mensajeria.o utilidades.o cartas.o -o servidor

cliente: cliente.o mensajeria.o cartas.o
	$(CC) $(CFLAGS) cliente.o mensajeria.o cartas.o -o cliente

servidor.o: servidor.c mensajeria.h utilidades.h cartas.h
	$(CC) $(CFLAGS) -c servidor.c

cliente.o: cliente.c mensajeria.h cartas.h
	$(CC) $(CFLAGS) -c cliente.c

mensajeria.o:	mensajeria.h mensajeria.c
	$(CC) $(CFLAGS) -c mensajeria.c

utilidades.o: utilidades.h utilidades.c
	$(CC) $(CFLAGS) -c utilidades.c

cartas.o: cartas.h cartas.c
	$(CC) $(CFLAGS) -c cartas.c

clean:
	$(RM) $(TARGET)
