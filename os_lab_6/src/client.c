#include "interprocess.h"

#define CLIENT_ID -1

unsigned children_count = 0;
unsigned capacity = INIT_CAPACITY;

int main(int argc, char const *argv[]) {

    Tree system;
    system = create_node(CLIENT_ID);

    void *context = zmq_ctx_new();
    if (context == NULL) {
        perror("context");
        exit(EXIT_FAILURE);
    }

    void **children_sockets = calloc(sizeof(void *), INIT_CAPACITY);
    id *children_ids = calloc(sizeof(id), INIT_CAPACITY);
    const char *arrow = "> ";
    size_t arrow_len = strlen(arrow);

    while (1) {
        if (write(1, arrow, arrow_len) <= 0) {
            perror("write");
            exit(EXIT_FAILURE);
        }
        command_t current = get_command();
        switch (current) {
        case PRINT:
            print_tree(system, 0);
            break;
        case EXIT:
            for (int i = 0; i < children_count; ++i) {
                send_exit(children_sockets[i]);
            }
            break;
        case CREATE:;
            const char *init_id_str = read_word();
            const char *parent_id_str = read_word();
            int init_id = atoi(init_id_str);
            if (init_id <= 0) {
                printf("Error: invalid node id (id should be an integer greater than 0).\n");
                break;
            }
            if (exists(system, init_id)) {
                printf("Error: already exists.\n");
                break;
            }
            int parent_id = atoi(parent_id_str);
            if (parent_id <= 0 && parent_id != CLIENT_ID) {
                printf("Error: invalid parent id (parent should be an integer greater than 0 or %d for root).\n", CLIENT_ID);
                break;
            }
            if (!exists(system, parent_id)) {
                printf("Error: there is no nodes with id = %d.\n", parent_id);
                break;
            }

            if (parent_id == CLIENT_ID) {
                int fork_val = fork();
                if (fork_val == 0)
                    create_worker(init_id);
                printf("Created node %d with PID %d\n", init_id, fork_val);

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

                rc = zmq_connect(children_sockets[children_count], portname_client(BASE_PORT + init_id));
                assert(rc == 0);

                children_ids[children_count++] = init_id;
                system = add_node(system, parent_id, init_id);
                break;
            } else {
                int replied = 0;
                for (int i = 0; i < children_count; ++i) {
                    send_create(children_sockets[i], init_id, parent_id);
                    if (!is_available_recv(children_sockets[i])) {
                        continue;
                    }
                    const char *reply = get_reply(children_sockets[i]);
                    if (strcmp(EMPTY_MSG, reply) != 0) {
                        printf("%s\n", reply);
                        replied = 1;
                        break;
                    }
                }
                if (replied == 0)
                    printf("Node %d seems to be unavailable\n", parent_id);
                else
                    system = add_node(system, parent_id, init_id);
            }
            break;

        case EXEC:;
            const char *target_id_str = read_word();
            int target_id = atoi(target_id_str);
            if (target_id <= 0) {
                printf("Error: invalid node id (id should be an integer greater than 0).\n");
                break;
            }
            if (!exists(system, target_id)) {
                printf("Error: this node does not exists.\n");
                break;
            }
            subcommand_t current_sub = get_subcommand();
            if (current_sub == UNDEFINED) {
                printf("Invalid subcommand!\n");
                break;
            }

            int replied = 0;
            for (int i = 0; i < children_count; ++i) {
                send_exec(children_sockets[i], current_sub, target_id);
                if (!is_available_recv(children_sockets[i])) {
                    continue;
                }
                char *reply = get_reply(children_sockets[i]);
                if (strcmp(EMPTY_MSG, reply) != 0) {
                    printf("%s\n", reply);
                    replied = 1;
                    break;
                }
            }
            if (replied == 0)
                printf("Node %d seems to be unavailable\n", target_id);

            break;
        case REMOVE:;
            const char *remove_id_str = read_word();
            int remove_id = atoi(remove_id_str);
            if (remove_id <= 0) {
                printf("Error: invalid node id (id should be an integer greater than 0).\n");
                break;
            }
            if (!exists(system, remove_id)) {
                printf("Error: this node does not exist.\n");
                break;
            }
            int i = in_list(children_ids, remove_id, children_count);
            if (i != -1) {
                shift_id(children_ids, i, children_count);
                send_exit(children_sockets[i]);
                zmq_close(children_sockets[i]);
                shift_void(children_sockets, i, children_count);
                children_count--;
                printf("Successfully removed node %d from system\n", remove_id);
            } else {
                replied = 0;
                for (int i = 0; i < children_count; ++i) {
                    send_remove(children_sockets[i], remove_id);
                    if (!is_available_recv(children_sockets[i])) {
                        continue;
                    }
                    char *reply = get_reply(children_sockets[i]);
                    if (strcmp(EMPTY_MSG, reply) != 0) {
                        printf("%s\n", reply);
                        replied = 1;
                        break;
                    }
                }
                if (replied == 0)
                    printf("Node %d seems to be unavailable, removed it from tree anyways\n", remove_id);
            }
            system = remove_node(system, remove_id);
            break;
        case PINGALL:;
            int not_replied = 0;
            int layer = 0;
            char *unavailable = (char *)calloc(sizeof(char), STR_LEN);
            for (int i = 0; i < children_count; ++i) {
                send_pingall(children_sockets[i], layer + 1);
                if (!is_available_recv_pingall(children_sockets[i], layer)) {
                    unavailable = strcat(unavailable, int_to_string(children_ids[i]));
                    unavailable = strcat(unavailable, " ");
                    not_replied++;
                    continue;
                }
                char *reply_child = get_reply(children_sockets[i]);
                if (strcmp(EMPTY_MSG, reply_child) != 0) {
                    unavailable = strcat(unavailable, reply_child);
                    replied = 1;
                    break;
                }
            }
            if (not_replied == 0 && replied == 0) {
                printf("Every process is available\n");
            } else {
                fprintf(stderr, "%s\n", unavailable);
            }
            break;
        default:
            printf("Error: invalid command.\n");
            break;
        }
        if (current == EXIT) {
            break;
        }
    }

    for (int i = 0; i < children_count; ++i) {
        zmq_close(children_sockets[i]);
    }
    zmq_ctx_destroy(context);

    return 0;
}