#include "interprocess.h"

void send_exec(void *socket, subcommand_t subcommand, id node_id) {
    command_t c = EXEC;
    zmq_msg_t command_msg;
    zmq_msg_init_size(&command_msg, sizeof(c));
    memcpy(zmq_msg_data(&command_msg), &c, sizeof(c));
    zmq_msg_send(&command_msg, socket, ZMQ_SNDMORE);
    zmq_msg_close(&command_msg);

    zmq_msg_t subcommand_msg;
    zmq_msg_init_size(&subcommand_msg, sizeof(subcommand_t));
    memcpy(zmq_msg_data(&subcommand_msg), &subcommand, sizeof(subcommand_t));
    zmq_msg_send(&subcommand_msg, socket, ZMQ_SNDMORE);
    zmq_msg_close(&subcommand_msg);

    zmq_msg_t id_msg;
    zmq_msg_init_size(&id_msg, sizeof(node_id));
    memcpy(zmq_msg_data(&id_msg), &node_id, sizeof(node_id));
    zmq_msg_send(&id_msg, socket, 0);
    zmq_msg_close(&id_msg);
}

void send_create(void *socket, id init_id, id parent_id) {
    command_t c = CREATE;
    zmq_msg_t command_msg;
    zmq_msg_init_size(&command_msg, sizeof(c));
    memcpy(zmq_msg_data(&command_msg), &c, sizeof(c));
    zmq_msg_send(&command_msg, socket, ZMQ_SNDMORE);
    zmq_msg_close(&command_msg);

    zmq_msg_t id_msg;
    zmq_msg_init_size(&id_msg, sizeof(init_id));
    memcpy(zmq_msg_data(&id_msg), &init_id, sizeof(init_id));
    zmq_msg_send(&id_msg, socket, ZMQ_SNDMORE);
    zmq_msg_close(&id_msg);

    zmq_msg_t parent_id_msg;
    zmq_msg_init_size(&parent_id_msg, sizeof(parent_id));
    memcpy(zmq_msg_data(&parent_id_msg), &parent_id, sizeof(parent_id));
    zmq_msg_send(&parent_id_msg, socket, 0);
    zmq_msg_close(&parent_id_msg);
}

void send_remove(void *socket, id remove_id) {
    command_t c = REMOVE;
    zmq_msg_t command_msg;
    zmq_msg_init_size(&command_msg, sizeof(c));
    memcpy(zmq_msg_data(&command_msg), &c, sizeof(c));
    zmq_msg_send(&command_msg, socket, ZMQ_SNDMORE);
    zmq_msg_close(&command_msg);

    zmq_msg_t id_msg;
    zmq_msg_init_size(&id_msg, sizeof(remove_id));
    memcpy(zmq_msg_data(&id_msg), &remove_id, sizeof(remove_id));
    zmq_msg_send(&id_msg, socket, 0);
    zmq_msg_close(&id_msg);
}

void send_exit(void *socket) {
    command_t c = EXIT;
    zmq_msg_t command_msg;
    zmq_msg_init_size(&command_msg, sizeof(c));
    memcpy(zmq_msg_data(&command_msg), &c, sizeof(c));
    zmq_msg_send(&command_msg, socket, 0);
    zmq_msg_close(&command_msg);
}

void send_pingall(void *socket, int layer) {
    command_t c = PINGALL;
    zmq_msg_t command_msg;
    zmq_msg_init_size(&command_msg, sizeof(c));
    memcpy(zmq_msg_data(&command_msg), &c, sizeof(c));
    zmq_msg_send(&command_msg, socket, ZMQ_SNDMORE);
    zmq_msg_close(&command_msg);

    zmq_msg_t l_msg;
    zmq_msg_init_size(&l_msg, sizeof(layer));
    memcpy(zmq_msg_data(&l_msg), &layer, sizeof(layer));
    zmq_msg_send(&l_msg, socket, 0);
    zmq_msg_close(&l_msg);
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

char *get_reply_pingall(void *socket) {
    int arg = 0;
    char *result = NULL;
    while (1) {
        zmq_msg_t part;
        int rec = zmq_msg_init(&part);
        assert(rec == 0);
        rec = zmq_msg_recv(&part, socket, 0);
        assert(rec != -1);
        switch (arg) {
        case 0:
            break;
        case 1:;
            size_t result_size = zmq_msg_size(&part);
            result = (char *)calloc(sizeof(char), result_size + 1);
            memcpy(result, zmq_msg_data(&part), result_size);
            break;
        }
        zmq_msg_close(&part);
        ++arg;
        if (!zmq_msg_more(&part))
            break;
    }

    return result;
}

void create_worker(id init_id) {
    const char *arg0 = SERVER_PATH;
    const char *arg1 = int_to_string(init_id);
    execl(SERVER_PATH, arg0, arg1, (char *)NULL);
}

int is_available_recv(void *socket) {
    zmq_pollitem_t items[1] = {{socket, 0, ZMQ_POLLIN, 0}};
    int rc = zmq_poll(items, 1, REQUEST_TIMEOUT);
    assert(rc != -1);
    if (items[0].revents & ZMQ_POLLIN)
        return 1;
    return 0;
}

int is_available_recv_pingall(void *socket, int layer) {
    zmq_pollitem_t items[1] = {{socket, 0, ZMQ_POLLIN, 0}};
    int rc = zmq_poll(items, 1, REQUEST_TIMEOUT - layer * ADDITIONAL_TIME);
    assert(rc != -1);
    if (items[0].revents & ZMQ_POLLIN)
        return 1;
    return 0;
}

int is_available_send(void *socket) {
    zmq_pollitem_t items[1] = {{socket, 0, ZMQ_POLLOUT, 0}};
    int rc = zmq_poll(items, 1, REQUEST_TIMEOUT);
    assert(rc != -1);
    if (items[0].revents & ZMQ_POLLOUT)
        return 1;
    return 0;
}

void shift_void(void **array, int pos, int capacity) {
    if (pos == capacity - 1) {
        array[pos] = NULL;
        return;
    }
    for (int i = pos; i < capacity - 1; ++i) {
        array[i] = array[i + 1];
    }
    array[capacity - 1] = NULL;
    return;
}

void shift_id(id *array, int pos, int capacity) {
    if (pos == capacity - 1) {
        array[pos] = 0;
        return;
    }
    for (int i = pos; i < capacity - 1; ++i) {
        array[i] = array[i + 1];
    }
    array[capacity - 1] = 0;
    return;
}

int in_list(id *array, id target, int capacity) {
    for (int i = 0; i < capacity; ++i) {
        if (array[i] == target)
            return i;
    }
    return -1;
}