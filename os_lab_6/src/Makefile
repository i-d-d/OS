CC = gcc
LD = gcc
LDFLAGS = -lzmq
CFLAGS = -Wall -pedantic -std=c99 -c

SRC_CLIENT = client.c 
OBJ_CLIENT = $(SRC_CLIENT: .c = .o)

SRC_SERVER = server.c
OBJ_SERVER = $(SRC_SERVER: .c = .o)

SRC_OTH = tree.c interface.c interprocess.c
OBJ_OTH = $(SRC_OTH: .c = .o)


all: object
	$(LD) $(OBJ_SERVER) $(OBJ_OTH) -o server $(LDFLAGS)
	$(LD) $(OBJ_CLIENT) $(OBJ_OTH) -o client $(LDFLAGS)

object: $(SRC_CLIENT) $(SRC_SERVER) $(SRC_OTH)
	$(CC) $(SRC_CLIENT) $(CFLAGS)
	$(CC) $(SRC_OTH) $(CFLAGS)
	$(CC) $(SRC_SERVER) $(CFLAGS)

remove: clean
	rm -rf server client

clean:
	rm -rf *.o 