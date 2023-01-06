#include "interprocess.h"

int main() {

    void *context = zmq_ctx_new();
    if (context == NULL) {
        perror("context");
        exit(EXIT_FAILURE);
    }
    void *responder = zmq_socket(context, ZMQ_REP);
    int rc = zmq_bind(responder, portname_server(BASE_PORT));
    assert(rc == 0);

    int game_number = 0;
    int available = 0;
    int *players = (int *)calloc(sizeof(int), NUMBER_OF_GAMES);
    int *can_join = (int *)calloc(sizeof(int), NUMBER_OF_GAMES);

    while (1) {
        int arg = 0;
        int number_of_players = 0;
        command_t current = UNKNOWN;
        while (1) {
            zmq_msg_t msg;
            int rec = zmq_msg_init(&msg);
            assert(rec == 0);
            rec = zmq_msg_recv(&msg, responder, 0);
            assert(rec != -1);
            switch (arg) {
            case 0:
                memcpy(&current, zmq_msg_data(&msg), sizeof(current));
                break;
            case 1:
                if (current == CREATE)
                    memcpy(&number_of_players, zmq_msg_data(&msg), sizeof(number_of_players));
                break;
            }
            zmq_msg_close(&msg);
            ++arg;
            if (!zmq_msg_more(&msg))
                break;
        }

        int reply;
        zmq_msg_t rep_msg;
        switch (current) {
        case CREATE:;
            if (game_number < NUMBER_OF_GAMES) {
                reply = BASE_PORT + 1;
                for (int i = 0; i < game_number; ++i) {
                    reply += players[i];
                }
                if (number_of_players != 1) {
                    available++;
                }
                players[game_number] = number_of_players;
                can_join[game_number++] = number_of_players - 1;
            } else {
                reply = 0;
            }

            zmq_msg_init_size(&rep_msg, sizeof(reply));
            memcpy(zmq_msg_data(&rep_msg), &reply, sizeof(reply));
            zmq_msg_send(&rep_msg, responder, 0);
            zmq_msg_close(&rep_msg);
            int fork_val = fork();
            if (fork_val == 0)
                create_game(reply, number_of_players);
            printf("%d\n", fork_val);
            break;
        case JOIN:;
            int reply;
            if (available != 0) {
                reply = BASE_PORT + 1;
                for (int i = 0; i < game_number; ++i) {
                    if (can_join[i] > 0) {
                        reply += players[i] - can_join[i];
                        if (--can_join[i] == 0)
                            available--;
                        break;
                    }
                    reply += players[i];
                }
            } else {
                reply = 0;
            }

            zmq_msg_init_size(&rep_msg, sizeof(reply));
            memcpy(zmq_msg_data(&rep_msg), &reply, sizeof(reply));
            zmq_msg_send(&rep_msg, responder, 0);
            zmq_msg_close(&rep_msg);
            break;
        case KILL:
            zmq_msg_init_size(&rep_msg, 0);
            zmq_msg_send(&rep_msg, responder, 0);
            zmq_msg_close(&rep_msg);
            zmq_close(responder);
            zmq_ctx_destroy(context);
            return 0;
        default:
            printf("Hmm\n");
            zmq_close(responder);
            zmq_ctx_destroy(context);
            return 0;
            break;
        }
    }
}
