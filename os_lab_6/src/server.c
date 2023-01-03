#include <sys/time.h>

#include "interprocess.h"

#define OFF 0
#define ON 1

#define INIT_CAPACITY 5

short timer = OFF;

unsigned children_count = 0;
unsigned capacity = INIT_CAPACITY;

int main(int argc, const char **argv) {
    void *context = zmq_ctx_new();
    if (context == NULL) {
        perror("context");
        exit(EXIT_FAILURE);
    }
    void *parent = zmq_socket(context, ZMQ_REP);
    if (parent == NULL) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    int self_id = atoi(argv[1]);
    int rc = zmq_bind(parent, portname_server(BASE_PORT + self_id));
    assert(rc == 0);

    void **children_sockets = calloc(sizeof(void *), INIT_CAPACITY);
    id *children_ids = calloc(sizeof(id), INIT_CAPACITY);

    double diff_sec = 0, diff_msec = 0;

    struct timeval start, finish;

    while (1) {
        command_t current = UNKNOWN;
        subcommand_t current_sub = UNDEFINED;
        int target_id = 0;
        int parent_id = 0;
        int arg = 0;
        int layer = 0;

        while (1) {
            zmq_msg_t part;
            int rec = zmq_msg_init(&part);
            assert(rec == 0);
            rec = zmq_msg_recv(&part, parent, 0);
            assert(rec != -1);

            switch (arg) {
            case 0:
                memcpy(&current, zmq_msg_data(&part), zmq_msg_size(&part));
                break;
            case 1:
                switch (current) {
                case EXEC:
                    memcpy(&current_sub, zmq_msg_data(&part), zmq_msg_size(&part));
                    break;
                case CREATE:
                    memcpy(&target_id, zmq_msg_data(&part), zmq_msg_size(&part));
                    break;
                case REMOVE:
                    memcpy(&target_id, zmq_msg_data(&part), zmq_msg_size(&part));
                    break;
                case PINGALL:
                    memcpy(&layer, zmq_msg_data(&part), zmq_msg_size(&part));
                    break;
                default:
                    break;
                }
                break;
            case 2:
                switch (current) {
                case EXEC:
                    memcpy(&target_id, zmq_msg_data(&part), zmq_msg_size(&part));
                    break;
                case CREATE:
                    memcpy(&parent_id, zmq_msg_data(&part), zmq_msg_size(&part));
                    break;
                default:
                    break;
                }
                break;
            default:
                printf("UNEXPECTED\n");
                exit(EXIT_FAILURE);
            }
            zmq_msg_close(&part);
            ++arg;
            if (!zmq_msg_more(&part))
                break;
        }
        if (current == EXIT) {
            for (int i = 0; i < children_count; ++i) {
                send_exit(children_sockets[i]);
            }
            break;
        }

        int replied = 0;
        int not_replied = 0;
        char *reply = (char *)calloc(sizeof(char), STR_LEN);

        if (current == CREATE) {
            if (parent_id == self_id) {
                int fork_val = fork();
                if (fork_val == 0)
                    create_worker(target_id);

                if (children_count >= capacity) {
                    capacity *= 2;
                    children_sockets = realloc(children_sockets, sizeof(void *) * capacity);
                    children_ids = realloc(children_ids, sizeof(id) * capacity);
                }
                children_sockets[children_count] = zmq_socket(context, ZMQ_REQ);
                int opt_val = 0;
                int rc = zmq_setsockopt(children_sockets[children_count], ZMQ_LINGER, &opt_val, sizeof(opt_val));
                assert(rc == 0);
                if (children_sockets[children_count] == NULL) {
                    perror("socket");
                    exit(EXIT_FAILURE);
                }

                rc = zmq_connect(children_sockets[children_count], portname_client(BASE_PORT + target_id));
                assert(rc == 0);

                children_ids[children_count++] = target_id;

                sprintf(reply, "Created node %d with PID %d", target_id, fork_val);
                replied = 1;

            } else {
                for (int i = 0; i < children_count; ++i) {
                    send_create(children_sockets[i], target_id, parent_id);
                    if (!is_available_recv(children_sockets[i])) {
                        continue;
                    }
                    const char *reply_child = get_reply(children_sockets[i]);
                    if (strcmp(EMPTY_MSG, reply_child) != 0) {
                        sprintf(reply, "%s", reply_child);
                        replied = 1;
                        break;
                    }
                }
            }
        }

        else if (current == EXEC) {

            if (target_id == self_id) {
                switch (current_sub) {
                case START:
                    gettimeofday(&start, NULL);
                    timer = ON;
                    break;
                case STOP:
                    if (timer == ON) {
                        gettimeofday(&finish, NULL);
                        timer = OFF;
                        diff_sec = finish.tv_sec - start.tv_sec;
                        diff_msec = (diff_sec * 1000) + (finish.tv_usec - start.tv_usec) / 1000;
                    }
                    break;
                case TIME:
                    if (timer == ON) {
                        gettimeofday(&finish, NULL);
                        diff_sec = finish.tv_sec - start.tv_sec;
                        diff_msec = (diff_sec * 1000) + (finish.tv_usec - start.tv_usec) / 1000;
                    }
                    break;
                default:
                    break;
                }

                char *result = message_prefix(self_id, current_sub);

                if (current_sub == TIME) {
                    const char *time_string = int_to_string((unsigned)diff_msec);
                    result = strcat(result, time_string);
                }
                strcpy(reply, result);
                replied = 1;
            } else {
                for (int i = 0; i < children_count; ++i) {
                    send_exec(children_sockets[i], current_sub, target_id);
                    if (!is_available_recv(children_sockets[i])) {
                        continue;
                    }
                    char *reply_child = get_reply(children_sockets[i]);
                    if (strcmp(EMPTY_MSG, reply_child) != 0) {
                        sprintf(reply, "%s", reply_child);
                        replied = 1;
                        break;
                    }
                }
            }
        }

        else if (current == REMOVE) {
            int i = in_list(children_ids, target_id, children_count);
            if (i != -1) {
                shift_id(children_ids, i, children_count);
                send_exit(children_sockets[i]);
                zmq_close(children_sockets[i]);
                shift_void(children_sockets, i, children_count);
                children_count--;
                sprintf(reply, "Successfully removed node %d from system", target_id);
                replied = 1;
            } else {
                for (int i = 0; i < children_count; ++i) {
                    send_remove(children_sockets[i], target_id);
                    if (!is_available_recv(children_sockets[i])) {
                        continue;
                    }
                    char *reply_child = get_reply(children_sockets[i]);
                    if (strcmp(EMPTY_MSG, reply_child) != 0) {
                        sprintf(reply, "%s", reply_child);
                        replied = 1;
                        break;
                    }
                }
            }
        }

        else if (current == PINGALL) {

            // zmq_msg_t pingall_response;
            // int rec = zmq_msg_init(&pingall_response);
            // assert(rec != -1);
            // zmq_msg_init_size(&pingall_response, strlen(EMPTY_MSG));
            // memcpy(zmq_msg_data(&pingall_response), EMPTY_MSG, strlen(EMPTY_MSG));
            // zmq_msg_send(&pingall_response, parent, ZMQ_SNDMORE);
            // zmq_msg_close(&pingall_response);

            for (int i = 0; i < children_count; ++i) {
                send_pingall(children_sockets[i], layer + 1);
                if (!is_available_recv_pingall(children_sockets[i], layer)) {
                    reply = strcat(reply, int_to_string(children_ids[i]));
                    reply = strcat(reply, " ");
                    not_replied++;
                    continue;
                }
                char *reply_child = get_reply(children_sockets[i]);
                if (strcmp(EMPTY_MSG, reply_child) != 0) {
                    reply = strcat(reply, reply_child);
                    replied = 1;
                    break;
                }
            }
        }

        if (replied == 0 && (current != PINGALL || (current == PINGALL && not_replied == 0)))
            strcpy(reply, EMPTY_MSG);
        size_t rep_len = strlen(reply) + 1;
        zmq_msg_t create_response;
        int rec = zmq_msg_init(&create_response);
        assert(rec != -1);
        zmq_msg_init_size(&create_response, rep_len);
        memcpy(zmq_msg_data(&create_response), reply, rep_len);
        zmq_msg_send(&create_response, parent, 0);
        zmq_msg_close(&create_response);
        free(reply);
    }

    zmq_close(parent);
    zmq_ctx_destroy(context);

    return 0;
}