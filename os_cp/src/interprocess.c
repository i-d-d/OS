#include "interprocess.h"

const char *int_to_string(unsigned a) {
    int x = a, i = 0;
    if (a == 0)
        return "0";
    while (x > 0) {
        x /= 10;
        i++;
    }
    char *result = (char *)calloc(sizeof(char), i + 1);
    while (i >= 1) {
        result[--i] = a % 10 + '0';
        a /= 10;
    }

    return result;
}

char *get_reply(void *socket) {
    zmq_msg_t reply;
    zmq_msg_init(&reply);
    zmq_msg_recv(&reply, socket, 0);
    size_t result_size = zmq_msg_size(&reply);

    char *result = (char *)calloc(sizeof(char), result_size + 1);
    memcpy(result, zmq_msg_data(&reply), result_size);
    zmq_msg_close(&reply);

    return result;
}

void send_guess_res(void *socket, char *log, int guessed) {
    zmq_msg_t command_msg;
    zmq_msg_init_size(&command_msg, sizeof(guessed));
    memcpy(zmq_msg_data(&command_msg), &guessed, sizeof(guessed));
    zmq_msg_send(&command_msg, socket, ZMQ_SNDMORE);
    zmq_msg_close(&command_msg);

    zmq_msg_t l_msg;
    zmq_msg_init_size(&l_msg, strlen(log));
    memcpy(zmq_msg_data(&l_msg), log, strlen(log) + 1);
    zmq_msg_send(&l_msg, socket, 0);
    zmq_msg_close(&l_msg);
}

const char *portname_client(unsigned short port) {
    const char *port_string = int_to_string(port);
    char *name = (char *)calloc(sizeof(char), strlen(CLIENT_ADRESS_PREFIX) + strlen(port_string) + 1);
    strcpy(name, CLIENT_ADRESS_PREFIX);
    strcpy(name + strlen(CLIENT_ADRESS_PREFIX) * sizeof(char), port_string);
    return name;
}

const char *portname_server(unsigned short port) {
    const char *port_string = int_to_string(port);
    char *name = (char *)calloc(sizeof(char), strlen(SERVER_ADRESS_PREFIX) + strlen(port_string) + 1);
    strcpy(name, SERVER_ADRESS_PREFIX);
    strcpy(name + strlen(SERVER_ADRESS_PREFIX) * sizeof(char), port_string);
    return name;
}

void create_game(int port, int number) {
    const char *arg0 = SERVER_PATH;
    const char *arg1 = int_to_string(port);
    const char *arg2 = int_to_string(number);
    execl(SERVER_PATH, arg0, arg1, arg2, NULL);
}

const char *read_word() {
    char *result = (char *)calloc(sizeof(char), STR_LEN);
    char current;
    int i = 0;
    TRY_READ(&current);
    while (current != ' ') {
        if (current == '\n' || current == '\0')
            break;
        result[i++] = current;
        TRY_READ(&current);
    }
    result = (char *)realloc(result, sizeof(char) * (strlen(result) + 1));
    return result;
}

command_t get_command() {
    const char *input = read_word();

    if (strcmp("create", input) == 0)
        return CREATE;

    if (strcmp("join", input) == 0)
        return JOIN;

    if (strcmp("exit", input) == 0)
        return EXIT;

    if (strcmp("kill", input) == 0)
        return KILL;

    return UNKNOWN;
}