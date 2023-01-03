#include "tree.h"

Node *create_node(id init_id) {
    Node *new_node = (Node *)malloc(sizeof(Node));
    new_node->node_id = init_id;
    new_node->children_count = 0;
    new_node->children_capacity = INIT_CAPACITY;
    new_node->children = (Node **)calloc(sizeof(Node *), INIT_CAPACITY);
    for (int i = 0; i < INIT_CAPACITY; ++i) {
        new_node->children[i] = NULL;
    }

    return new_node;
}

int exists(Node *root, id target_id) {
    if (root == NULL)
        return 0;
    if (root->node_id == target_id)
        return 1;

    int result = 0;
    for (int i = 0; i < root->children_count; i++)
        result |= exists(root->children[i], target_id);

    return result;
}

Node *add_node(Node *root, id parent_id, id new_id) {
    if (root->node_id == parent_id) {
        if (root->children_count >= root->children_capacity) {
            root->children_capacity *= 2;
            root->children = (Node **)realloc(root->children, sizeof(Node *) * root->children_capacity);
        }
        root->children[(root->children_count)++] = create_node(new_id);
        return root;
    }

    if (root->children_count == 0)
        return root;

    for (int i = 0; i < root->children_count; ++i)
        root->children[i] = add_node(root->children[i], parent_id, new_id);

    return root;
}

Node *delete_tree(Node *root) {
    if (root->children_count == 0) {
        free(root->children);
        free(root);
        return NULL;
    }

    for (int i = 0; i < root->children_count; ++i)
        root->children[i] = delete_tree(root->children[i]);

    return NULL;
}

Node *remove_node(Node *root, id target_id) {
    if (root->node_id == target_id)
        return delete_tree(root);

    if (root->children_count == 0)
        return root;

    Node *result;

    for (int i = 0; i < root->children_count; i++) {
        result = root->children[i] = remove_node(root->children[i], target_id);
        if (result == NULL) {
            for (int j = i; j < root->children_count - 1; ++j) {
                root->children[j] = root->children[j + 1];
            }
            root->children[root->children_count - 1] = NULL;
            root->children_count--;
            break;
        }
    }
    return root;
}

void print_tree(const Node *root, unsigned depth) {
    if (root == NULL) {
        return;
    }
    for (int i = 0; i < depth; ++i)
        printf("\t");
    if (root->node_id == (id)-1)
        printf("-1\n");
    else
        printf("%d\n", root->node_id);
    for (int i = 0; i < root->children_count; ++i) {
        print_tree(root->children[i], depth + 1);
    }
}