#ifndef __INTERPROCESS_H__
#define __INTERPROCESS_H__

#include <assert.h>
#include <zmq.h>

#include "interface.h"
#include "tree.h"

#define SERVER_PATH "./server"

#define REQUEST_TIMEOUT 2000
#define ADDITIONAL_TIME 100

#define EMPTY_MSG ""

void send_exec(void *socket, subcommand_t subcommand, id node_id);
void send_create(void *socket, id init_id, id parent_id);
void send_exit(void *socket);
void send_pingall(void *socket, int layer);
void send_remove(void *socket, id remove_id);
char *get_reply(void *socket);
char *get_reply_pingall(void *socket);
void create_worker(id init_id);
int is_available_recv(void *socket);
int is_available_recv_pingall(void *socket, int additional);
int is_available_send(void *socket);

void shift_void(void **array, int pos, int capacity);
void shift_id(id *array, int pos, int capacity);
int in_list(id *array, id target, int capacity);

#endif