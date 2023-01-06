CC = gcc
LD = gcc
LDFLAGS = -lzmq
CFLAGS = -Wall -pedantic -std=c99 -c

SRC_CLIENT = client.c 
OBJ_CLIENT = $(SRC_CLIENT: .c = .o)

SRC_SERVER = server.c
OBJ_SERVER = $(SRC_SERVER: .c = .o)

SRC_GAME = game.c
OBJ_GAME = $(SRC_GAME: .c = .o)

SRC_OTH = interprocess.c 
OBJ_OTH = $(SRC_OTH: .c = .o)


all: object
	$(LD) $(OBJ_SERVER) $(OBJ_OTH) -o server $(LDFLAGS)
	$(LD) $(OBJ_CLIENT) $(OBJ_OTH) -o client $(LDFLAGS)
	$(LD) $(OBJ_GAME) $(OBJ_OTH) -o game $(LDFLAGS)

object: $(SRC_CLIENT) $(SRC_SERVER) $(SRC_GAME) $(SRC_OTH)
	$(CC) $(SRC_CLIENT) $(CFLAGS)
	$(CC) $(SRC_GAME) $(CFLAGS)
	$(CC) $(SRC_OTH) $(CFLAGS)
	$(CC) $(SRC_SERVER) $(CFLAGS)

remove: clean
	rm -rf server client game

clean:
	rm -rf *.o 