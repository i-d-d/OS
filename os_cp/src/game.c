#include <time.h>

#include "interprocess.h"

const char *player_name(int id) {
    const char *id_str = int_to_string(id + 1);
    char *name = (char *)calloc(sizeof(char), strlen("Player ") + strlen(id_str) + 1);
    strcpy(name, "Player ");
    strcpy(name + strlen("Player ") * sizeof(char), id_str);
    return name;
}

const char *check_guess(long target, long guess) {
    int b = 0, c = 0;
    const char *target_str = int_to_string(target);
    const char *guess_str = int_to_string(guess);
    for (int i = 0; i < 5; ++i) {
        if (target_str[i] == guess_str[i])
            b++;
        else {
            int d = 0;
            for (int k = 0; k < i; ++k) {
                if (guess_str[k] == guess_str[i])
                    d++;
            }
            int occ = 0;
            for (int j = 0; j < 5; ++j) {
                if (target_str[j] == guess_str[i])
                    occ++;
            }
            if (occ > d)
                c++;
        }
    }
    char *result = (char *)calloc(sizeof(char), 5);
    sprintf(result, "%db%dc", b, c);
    return result;
}

int main(int argc, char const *argv[]) {
    int port = atoi(argv[1]);
    int player_number = atoi(argv[2]);

    void *context = zmq_ctx_new();
    if (context == NULL) {
        perror("context");
        exit(EXIT_FAILURE);
    }

    void **sockets = calloc(sizeof(void *), player_number);
    for (int i = 0; i < player_number; ++i) {
        sockets[i] = zmq_socket(context, ZMQ_REQ);
        int opt_val = 0;
        int rc = zmq_setsockopt(sockets[i], ZMQ_LINGER, &opt_val, sizeof(opt_val));
        assert(rc == 0);
        if (sockets[i] == NULL) {
            perror("socket");
            exit(EXIT_FAILURE);
        }

        rc = zmq_connect(sockets[i], portname_client(port + i));
        assert(rc == 0);

        zmq_msg_t rep_msg;
        zmq_msg_init_size(&rep_msg, sizeof(i));
        memcpy(zmq_msg_data(&rep_msg), &i, sizeof(i));
        zmq_msg_send(&rep_msg, sockets[i], 0);
        zmq_msg_close(&rep_msg);

        zmq_msg_t msg;
        rc = zmq_msg_init(&msg);
        assert(rc == 0);
        rc = zmq_msg_recv(&msg, sockets[i], 0);
        assert(rc != -1);
        zmq_msg_close(&msg);
    }
    for (int i = 0; i < player_number; ++i) {
        zmq_msg_t rep_msg;
        const char *greetings = "The game begins!";
        zmq_msg_init_size(&rep_msg, strlen(greetings));
        memcpy(zmq_msg_data(&rep_msg), greetings, strlen(greetings) + 1);
        zmq_msg_send(&rep_msg, sockets[i], 0);
        zmq_msg_close(&rep_msg);

        zmq_msg_t msg;
        int rc = zmq_msg_init(&msg);
        assert(rc == 0);
        rc = zmq_msg_recv(&msg, sockets[i], 0);
        assert(rc != -1);
        zmq_msg_close(&msg);
    }

    srand((unsigned)time(NULL));
    long number = rand() % 900000 + 10000;
    char *guesses = (char *)calloc(sizeof(char), STR_LEN_LONG);
    guesses = strcat(guesses, "All guesses:\n");
    int guessed = 0;
    int winner = 0;
    while (!guessed) {
        for (int i = 0; i < player_number; ++i) {
            send_guess_res(sockets[i], guesses, guessed);
            long guess;
            zmq_msg_t msg;
            int rc = zmq_msg_init(&msg);
            assert(rc == 0);
            rc = zmq_msg_recv(&msg, sockets[i], 0);
            assert(rc != -1);
            memcpy(&guess, zmq_msg_data(&msg), sizeof(guess));
            zmq_msg_close(&msg);
            if (guessed)
                continue;

            const char *check = check_guess(number, guess);

            guesses = strcat(guesses, player_name(i));
            guesses = strcat(guesses, " guessed ");
            guesses = strcat(guesses, int_to_string(guess));
            guesses = strcat(guesses, ". Result: ");
            guesses = strcat(guesses, check);
            guesses = strcat(guesses, "\n");

            if (strcmp(check, "5b0c") == 0) {
                guesses = strcat(guesses, player_name(i));
                guesses = strcat(guesses, " is the winner!");
                guessed = 1;
                winner = i;
            }
        }
    }
    for (int i = 0; i <= winner; ++i) {
        send_guess_res(sockets[i], guesses, guessed);
        zmq_msg_t msg;
        int rc = zmq_msg_init(&msg);
        assert(rc == 0);
        rc = zmq_msg_recv(&msg, sockets[i], 0);
        assert(rc != -1);
        zmq_msg_close(&msg);
    }

    for (int i = 0; i < player_number; ++i) {
        zmq_close(sockets[i]);
    }
    zmq_ctx_destroy(context);

    return 0;
}
