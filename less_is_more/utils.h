#ifndef UTILS_H
#define UTILS_H

#include "stdio.h"
#include <stdlib.h>
#include <string.h>

#ifdef _cplusplus
extern "C" {
#endif

#define MAX(a, b)                                                                                                      \
    ({                                                                                                                 \
        __typeof__(a) _a = (a);                                                                                        \
        __typeof__(b) _b = (b);                                                                                        \
        _a > _b ? _a : _b;                                                                                             \
    })

#define MIN(a, b)                                                                                                      \
    ({                                                                                                                 \
        __typeof__(a) _a = (a);                                                                                        \
        __typeof__(b) _b = (b);                                                                                        \
        _a > _b ? _b : _a;                                                                                             \
    })

char *shift(int *argc, char ***argv) {
    if (*argc <= 0) {
        fprintf(stderr, "ERROR: `shift` requested with non-positive argc\n");
        exit(1);
    }
    (*argc)--;
    return *((*argv)++);
}

void combine_charp(const char *str1, const char *str2, char **combined) {
    size_t length = strlen(str1) + strlen(str2) + 1;
    *combined = (char *)malloc(length);

    if (*combined == NULL) {
        fprintf(stderr,"Failed to allocate memory");
    }

    strcpy(*combined, str1);
    strcat(*combined, str2);
}

typedef struct {
    int *data;
    size_t size;
    size_t capacity;
} Ivector;

Ivector *init_Ivector(size_t init_cap) {
    Ivector *vector = (Ivector *)malloc(sizeof(Ivector));
    if (vector == NULL) {
        fprintf(stderr, "ERROR: Ivector_init malloc failed\n");
        exit(1);
    }

    vector->data = (int *)malloc(sizeof(int) * init_cap);
    vector->size = 0;
    vector->capacity = init_cap;

    return vector;
}

void resize_Ivector(Ivector *vector, size_t new_capacity) {
    vector->data = (int *)realloc(vector->data, sizeof(int) * new_capacity);

    vector->capacity = new_capacity;
}

void push_back_Ivector(Ivector *vector, int val) {
    if (vector->size >= vector->capacity) {
        resize_Ivector(vector, vector->size * 2);
    }
    vector->data[vector->size++] = val;
}

int pop_back_Ivector(Ivector *vector) {
    if (vector->size == 0) {
        fprintf(stderr, "ERROR: Ivector_pop_back cannot pop from empty vector\n");
        exit(1);
    }

    int val = vector->data[vector->size - 1];
    vector->size--;
    return val;
}

int get_Ivector(Ivector *vector, size_t pos) {
    if (pos >= vector->size) {
        fprintf(stderr, "ERROR: Ivector_get index %zu out of bounds (%zu)\n", pos, vector->size);
        exit(1);
    }
    return vector->data[pos];
}

void reset_Ivector(Ivector *vector) {
    free(vector->data);
    vector->size = 0;
    vector->capacity = 0;
}

#ifdef _cplusplus
};
#endif
#endif
