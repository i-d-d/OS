#include "interprocess.h"

void play_game(void *socket) {
    int id;
    zmq_msg_t msg;
    int rc = zmq_msg_init(&msg);
    assert(rc == 0);
    rc = zmq_msg_recv(&msg, socket, 0);
    assert(rc != -1);
    memcpy(&id, zmq_msg_data(&msg), sizeof(id));
    zmq_msg_close(&msg);

    printf("You will be playing as Player %d\n", id + 1);
    printf("Now let's wait for others\n");

    rc = zmq_msg_init(&msg);
    assert(rc == 0);
    rc = zmq_msg_send(&msg, socket, 0);
    assert(rc != -1);
    zmq_msg_close(&msg);

    rc = zmq_msg_init(&msg);
    assert(rc == 0);
    rc = zmq_msg_recv(&msg, socket, 0);
    assert(rc != -1);
    size_t result_size = zmq_msg_size(&msg);

    char *text = (char *)calloc(sizeof(char), result_size + 1);
    memcpy(text, zmq_msg_data(&msg), result_size);
    zmq_msg_close(&msg);
    printf("%s\n", text);

    rc = zmq_msg_init(&msg);
    assert(rc == 0);
    rc = zmq_msg_send(&msg, socket, 0);
    assert(rc != -1);
    zmq_msg_close(&msg);

    while (1) {
        int arg = 0;
        int type = -1;
        char *guesses = (char *)calloc(sizeof(char), STR_LEN_LONG);
        while (1) {
            zmq_msg_t msg;
            int rec = zmq_msg_init(&msg);
            assert(rec == 0);
            rec = zmq_msg_recv(&msg, socket, 0);
            assert(rec != -1);
            switch (arg) {
            case 0:
                memcpy(&type, zmq_msg_data(&msg), sizeof(type));
                break;
            case 1:
                memcpy(guesses, zmq_msg_data(&msg), zmq_msg_size(&msg));
                break;
            }
            zmq_msg_close(&msg);
            ++arg;
            if (!zmq_msg_more(&msg))
                break;
        }
        printf("\n%s", guesses);
        if (type == 0) {
            printf("\nMake your move!\n");
            long guess;
            while (1) {
                const char *guess_str = read_word();
                guess = atoi(guess_str);
                if (guess > 99999 || guess < 10000) {
                    printf("Number should contain exactly 5 digits\n");
                    continue;
                }
                break;
            }
            zmq_msg_t g_msg;
            rc = zmq_msg_init(&g_msg);
            assert(rc == 0);
            zmq_msg_init_size(&g_msg, sizeof(guess));
            memcpy(zmq_msg_data(&g_msg), &guess, sizeof(guess));
            rc = zmq_msg_send(&g_msg, socket, 0);
            printf("sent!\n");
            assert(rc != -1);
            zmq_msg_close(&g_msg);
        } else {
            zmq_msg_t g_msg;
            rc = zmq_msg_init(&g_msg);
            assert(rc == 0);
            rc = zmq_msg_send(&g_msg, socket, 0);
            assert(rc != -1);
            zmq_msg_close(&g_msg);
            break;
        }
    }
    zmq_close(socket);
}

int main() {
    command_t c;
    void *context = zmq_ctx_new();
    if (context == NULL) {
        perror("context");
        exit(EXIT_FAILURE);
    }
    void *client = zmq_socket(context, ZMQ_REQ);
    if (client == NULL) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int rc = zmq_connect(client, portname_client(BASE_PORT));
    assert(rc == 0);
    while (1) {
        c = get_command();
        zmq_msg_t req_msg;
        zmq_msg_t reply;
        int rep_val;
        switch (c) {
        case EXIT:
            break;
        case CREATE:;
            const char *number_str = read_word();
            int number = atoi(number_str);
            if (number <= 0) {
                printf("Players number should be a positive integer number\n");
                continue;
            }
            zmq_msg_init_size(&req_msg, sizeof(c));
            memcpy(zmq_msg_data(&req_msg), &c, sizeof(c));
            zmq_msg_send(&req_msg, client, ZMQ_SNDMORE);
            zmq_msg_close(&req_msg);

            zmq_msg_init_size(&req_msg, sizeof(number));
            memcpy(zmq_msg_data(&req_msg), &number, sizeof(number));
            zmq_msg_send(&req_msg, client, 0);
            zmq_msg_close(&req_msg);

            zmq_msg_init(&reply);
            zmq_msg_recv(&reply, client, 0);
            memcpy(&rep_val, zmq_msg_data(&reply), sizeof(rep_val));
            zmq_msg_close(&reply);

            if (rep_val == 0) {
                printf("Sorry, server is already full, try restarting it\n");
                zmq_close(client);
                zmq_ctx_destroy(context);
                return 0;
            } else {
                zmq_close(client);
                client = zmq_socket(context, ZMQ_REP);
                rc = zmq_bind(client, portname_server(rep_val));
                assert(rc == 0);
                play_game(client);
                zmq_ctx_destroy(context);
                return 0;
            }

            break;
        case JOIN:
            zmq_msg_init_size(&req_msg, sizeof(c));
            memcpy(zmq_msg_data(&req_msg), &c, sizeof(c));
            zmq_msg_send(&req_msg, client, 0);
            zmq_msg_close(&req_msg);

            zmq_msg_t rep;
            zmq_msg_init(&rep);
            zmq_msg_recv(&rep, client, 0);

            memcpy(&rep_val, zmq_msg_data(&rep), sizeof(rep_val));
            zmq_msg_close(&rep);
            if (rep_val == 0) {
                printf("Sorry, no available games for you\n");
                zmq_close(client);
                zmq_ctx_destroy(context);
                return 0;
            } else {
                zmq_close(client);
                client = zmq_socket(context, ZMQ_REP);
                rc = zmq_bind(client, portname_server(rep_val));
                assert(rc == 0);
                play_game(client);
                zmq_ctx_destroy(context);
                return 0;
            }
            break;
        case KILL:
            zmq_msg_init_size(&req_msg, sizeof(c));
            memcpy(zmq_msg_data(&req_msg), &c, sizeof(c));
            zmq_msg_send(&req_msg, client, 0);
            zmq_msg_close(&req_msg);

            zmq_msg_init(&reply);
            zmq_msg_recv(&reply, client, 0);
            c = EXIT;
            break;

        default:
            printf("Invalid command\n");
            continue;
            break;
        }
        if (c == EXIT)
            break;
    }
    zmq_close(client);
    zmq_ctx_destroy(context);
}