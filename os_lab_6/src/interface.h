#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CLIENT_ADRESS_PREFIX "tcp://localhost:"
#define SERVER_ADRESS_PREFIX "tcp://*:"

#define BASE_PORT 5555
#define STR_LEN 64

#define TRY_READ(C)            \
    if (read(0, (C), 1) < 0) { \
        perror("read");        \
        exit(EXIT_FAILURE);    \
    }

typedef enum { EXIT = 0,
               CREATE,
               REMOVE,
               EXEC,
               PINGALL,
               PRINT,
               UNKNOWN } command_t;

typedef enum { START = 0,
               STOP,
               TIME,
               UNDEFINED } subcommand_t;

const char *read_word();
command_t get_command();
subcommand_t get_subcommand();

void print_help();

const char *int_to_string(unsigned a);
const char *portname_client(unsigned short port);
const char *portname_server(unsigned short port);

char *message_prefix(unsigned node_id, subcommand_t sub);

#endif