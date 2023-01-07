#ifndef __INTERPROCESS_H__
#define __INTERPROCESS_H__

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <zmq.h>

#define TRY_READ(C)            \
    if (read(0, (C), 1) < 0) { \
        perror("read");        \
        exit(EXIT_FAILURE);    \
    }

#define SERVER_PATH "./game"

#define REQUEST_TIMEOUT 2000
#define ADDITIONAL_TIME 100

#define NUMBER_OF_GAMES 10

#define EMPTY_MSG ""

#define CLIENT_ADRESS_PREFIX "tcp://localhost:"
#define SERVER_ADRESS_PREFIX "tcp://*:"

#define BASE_PORT 5050
#define STR_LEN 64
#define STR_LEN_LONG 2048

typedef enum _command {
    CREATE = 0,
    JOIN,
    EXIT,
    KILL,
    UNKNOWN
} command_t;

int is_available_recv(void *socket);
int is_available_send(void *socket);

const char *int_to_string(unsigned a);
const char *portname_client(unsigned short port);
const char *portname_server(unsigned short port);

void create_game(int port, int number);

const char *read_word();
command_t get_command();

void send_guess_res(void *socket, char *log, int guessed);

#endif