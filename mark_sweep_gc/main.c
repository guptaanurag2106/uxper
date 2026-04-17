#include <stdio.h>

#include "heap.h"

typedef struct Node {
    int value;
    struct Node *left;
    struct Node *right;
} Node;

static Node *alloc_node(int value, const char *name) {
    Node *n = (Node *)heap_alloc(sizeof(Node), name);
    n->value = value;
    n->left = NULL;
    n->right = NULL;
    return n;
}

int main(void) {
    if (heap_init_stack_bounds() != 0) {
        fprintf(stderr, "failed to initialize stack bounds\n");
        return 1;
    }

    printf("\n[Test 1] basic roots\n");
    int *a = heap_alloc(sizeof(int), "int_a");
    volatile float *b = heap_alloc(sizeof(float), "float_b");

    *a = 2;
    *b = 2.2f;
    /* printf("&a -> %p a->%p %d, &b -> %p b->%p %f\n", (void *)&a, (void *)a,
     * *a, */
    /*        (void *)&b, (void *)b, *b); */

    heap_info();
    printf("Collecting with a = NULL (expect a collected, b alive)\n");
    a = NULL;
    heap_collect();
    heap_info();

    printf("\n[Test 2] tree reachability\n");
    Node *root = alloc_node(1, "root");
    root->left = alloc_node(2, "root_left");
    root->right = alloc_node(3, "root_right");
    root->left->left = alloc_node(4, "root_left_left");
    root->left->right = alloc_node(5, "root_left_right");
    root->right->left = alloc_node(6, "root_right_left");
    root->right->right = alloc_node(7, "root_right_right");

    printf("Collecting with root alive (expect tree alive)\n");
    heap_collect();
    heap_info();

    printf("Collecting with root = NULL (expect whole tree collected)\n");
    root = NULL;
    heap_collect();
    heap_info();

    printf("\n[Test 3] alias root\n");
    int *x = heap_alloc(sizeof(int), "x");
    int *x_alias = x;
    *x = 42;
    printf("x -> %p, x_alias -> %p\n", (void *)x, (void *)x_alias);

    printf("Collecting with x = NULL but alias alive (expect x alive)\n");
    x = NULL;
    heap_collect();
    heap_info();

    printf("Collecting with alias = NULL (expect x collected)\n");
    x_alias = NULL;
    heap_collect();
    heap_info();

    printf("\n[Test 4] Cyclic reference check\n");
    Node *a1 = alloc_node(1, "a1");
    Node *b1 = alloc_node(2, "b1");

    a1->left = b1;
    b1->left = a1;
    a1 = NULL;
    b1 = NULL;
    heap_collect();
    heap_info();

    return 0;
}
