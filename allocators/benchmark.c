#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define ARENA_IMPLEMENTATION
#include "arena.h"
#define POOL_ALLOC_IMPLEMENTATION
#include "pool_alloc.h"

#define POOL_ALLOC_IMPLEMENTATION
#define ARENA_IMPLEMENTATION

#define N 10000000
#define OBJ_SIZE 64

typedef struct {
    uint8_t data[OBJ_SIZE];
} Object;

static double now_seconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

void bench_tight_loop(bool print) {
    if (print) printf("=== Tight alloc/free loop (%d iters) ===\n", N);

    {
        double t0 = now_seconds();
        for (int i = 0; i < N; ++i) {
            void *p = malloc(OBJ_SIZE);
            free(p);
        }
        double t1 = now_seconds();
        if (print) printf("malloc/free:  %.6f sec\n", t1 - t0);
    }

    {
        Pool *pool = pool_create(N, OBJ_SIZE);
        double t0 = now_seconds();
        for (int i = 0; i < N; ++i) {
            void *p = pool_alloc(pool);
            pool_free(pool, p);
        }
        double t1 = now_seconds();
        if (print) printf("pool:         %.6f sec\n", t1 - t0);
        pool_destroy(pool);
    }

    {
        Arena arena = arena_create(OBJ_SIZE);
        double t0 = now_seconds();
        for (int i = 0; i < N; ++i) {
            void *p = arena_alloc(&arena, OBJ_SIZE);
            arena_clear(&arena);
            (void)p;
        }
        double t1 = now_seconds();
        if (print) printf("arena:        %.6f sec\n", t1 - t0);
        arena_destroy(&arena);
    }

    if (print) printf("\n");
}

void bench_batch(bool print) {
    if (print) printf("=== Batch allocate/free (%d objects) ===\n", N);

    void **ptrs = malloc(sizeof(void *) * N);

    {
        double t0 = now_seconds();
        for (int i = 0; i < N; ++i) ptrs[i] = malloc(OBJ_SIZE);

        for (int i = 0; i < N; ++i) free(ptrs[i]);

        double t1 = now_seconds();
        if (print) printf("malloc batch: %.6f sec\n", t1 - t0);
    }

    {
        Pool *pool = pool_create(N, OBJ_SIZE);

        double t0 = now_seconds();
        for (int i = 0; i < N; ++i) ptrs[i] = pool_alloc(pool);

        for (int i = 0; i < N; ++i) pool_free(pool, ptrs[i]);

        double t1 = now_seconds();
        if (print) printf("pool batch:   %.6f sec\n", t1 - t0);

        pool_destroy(pool);
    }

    {
        Arena arena = arena_create((size_t)N * OBJ_SIZE);

        double t0 = now_seconds();
        for (int i = 0; i < N; ++i) ptrs[i] = arena_alloc(&arena, OBJ_SIZE);

        arena_clear(&arena);

        double t1 = now_seconds();
        if (print) printf("arena batch:  %.6f sec\n", t1 - t0);

        arena_destroy(&arena);
    }

    free(ptrs);
    if (print) printf("\n");
}

void bench_touch(bool print) {
    if (print) printf("=== Allocate + write (%d objects) ===\n", N);

    void **ptrs = malloc(sizeof(void *) * N);

    {
        double t0 = now_seconds();
        for (int i = 0; i < N; ++i) {
            ptrs[i] = malloc(OBJ_SIZE);
            ((Object *)ptrs[i])->data[0] = (uint8_t)i;
        }
        for (int i = 0; i < N; ++i) free(ptrs[i]);
        double t1 = now_seconds();
        if (print) printf("malloc touch: %.6f sec\n", t1 - t0);
    }

    {
        Pool *pool = pool_create(N, OBJ_SIZE);

        double t0 = now_seconds();
        for (int i = 0; i < N; ++i) {
            ptrs[i] = pool_alloc(pool);
            ((Object *)ptrs[i])->data[0] = (uint8_t)i;
        }
        for (int i = 0; i < N; ++i) pool_free(pool, ptrs[i]);
        double t1 = now_seconds();
        if (print) printf("pool touch:   %.6f sec\n", t1 - t0);

        pool_destroy(pool);
    }

    {
        Arena arena = arena_create((size_t)N * OBJ_SIZE);

        double t0 = now_seconds();
        for (int i = 0; i < N; ++i) {
            ptrs[i] = arena_alloc(&arena, OBJ_SIZE);
            ((Object *)ptrs[i])->data[0] = (uint8_t)i;
        }
        arena_clear(&arena);
        double t1 = now_seconds();
        if (print) printf("arena touch:  %.6f sec\n", t1 - t0);

        arena_destroy(&arena);
    }

    free(ptrs);
    if (print) printf("\n");
}

int main(void) {
    // warmup
    bench_tight_loop(false);
    bench_batch(false);
    bench_touch(false);

    bench_tight_loop(true);
    bench_batch(true);
    bench_touch(true);
    return 0;
}
