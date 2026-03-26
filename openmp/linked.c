#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef N
#define N 500
#endif
#ifndef FS
#define FS 1000000
#endif

struct node {
    int data;
    unsigned long long fibdata;
    struct node *next;
};

int fib(int n) {
    int x, y;
    if (n < 2) {
        return (n);
    } else {
        x = fib(n - 1);
        y = fib(n - 2);
        return (x + y);
    }
}

unsigned long long fib2(int n) {
    unsigned long long a = 1, b = 1;
    unsigned long long c;
    for (int i = 1; i <= n - 2; i++) {
        c = a + b;
        a = b;
        b = c;
    }
    return c;
}

void processwork(struct node *p) {
    int n;
    n = p->data;
    p->fibdata = fib2(n);
}

struct node *init_list(struct node *p) {
    int i;
    struct node *head = NULL;
    struct node *temp = NULL;

    head = (struct node *)malloc(sizeof(struct node));
    p = head;
    p->data = FS;
    p->fibdata = 0;
    for (i = 0; i < N; i++) {
        temp = (struct node *)malloc(sizeof(struct node));
        p->next = temp;
        p = temp;
        p->data = FS + i + 1;
        p->fibdata = i + 1;
    }
    p->next = NULL;
    return head;
}

int main(int argc, char *argv[]) {
    double start, end;
    struct node *p = NULL;
    struct node *temp = NULL;
    struct node *head = NULL;

    printf("Process linked list\n");
    printf(
        "  Each linked list node will be processed by function "
        "'processwork()'\n");
    printf(
        "  Each ll node will compute %d fibonacci numbers beginning with %d\n",
        N, FS);

    p = init_list(p);
    head = p;

    // 1000000 : 14197223477820724411
    // 1000001 : 2756670985995446685
    // 1000002 : 16953894463816171096
    // 1000003 : 1263821376102066165
    // 1000004 : 18217715839918237261
    // 1000005 : 1034793142310751810
    start = omp_get_wtime();
    {
        while (p != NULL) {
            processwork(p);
            p = p->next;
        }
    }
    end = omp_get_wtime();
    printf("Time 1 %fs\n", end - start);

    // New
    p = head;
    struct node *nodes[N + 1][8] = {0};
    int i = 0;
    while (p != NULL) {
        nodes[i++][0] = p;
        p = p->next;
    }
    start = omp_get_wtime();
#pragma omp parallel for schedule(dynamic, 8)
    for (int x = 0; x <= N; x++) {
        processwork(nodes[x][0]);
    }
    end = omp_get_wtime();
    printf("Time 2 %fs\n", end - start);

    p = head;
    start = omp_get_wtime();
#pragma omp parallel
#pragma omp single
    {
        while (p) {
#pragma omp task firstprivate(p)
            processwork(p);
            p = p->next;
        }
    }
    end = omp_get_wtime();
    printf("Time 3 %fs\n", end - start);

    p = head;
    while (p != NULL) {
        printf("%d : %llu\n", p->data, p->fibdata);
        temp = p->next;
        free(p);
        p = temp;
    }
    free(p);

    // Time 1 0.813658s
    // Time 2 0.306247s
    // Time 3 0.303357s

    return 0;
}
