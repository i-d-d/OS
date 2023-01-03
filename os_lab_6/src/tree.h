#ifndef __TREE_H__
#define __TREE_H__

#include <stdio.h>
#include <stdlib.h>

#define INIT_CAPACITY 5

typedef unsigned short id;

typedef struct _node {
    id node_id;
    struct _node **children;
    unsigned children_count;
    unsigned children_capacity;
} Node;

typedef Node *Tree;

Node *create_node(id init_id);
int exists(Node *root, id target_id);
Node *add_node(Node *root, id parent_id, id init_id);
Node *remove_node(Node *root, id init_id);
void print_tree(const Node *root, unsigned depth);

#endif