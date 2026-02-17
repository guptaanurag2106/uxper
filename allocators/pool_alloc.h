#ifndef POOL_ALLOC_H_
#define POOL_ALLOC_H_

#include <assert.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef POOL_ALLOC_DEF
#ifdef POOL_ALLOC_IMPLEMENTATION
#define POOL_ALLOC_DEF
#else
#define POOL_ALLOC_DEF extern
#endif
#endif

#ifndef POOL_ALLOC_BACKEND_MALLOC
#define POOL_ALLOC_BACKEND_MALLOC malloc
#endif

#ifndef POOL_ALLOC_BACKEND_FREE
#define POOL_ALLOC_BACKEND_FREE free
#endif

#define POOL_CACHELINE_SIZE 64

typedef struct ListNode {
    struct ListNode *next;
} ListNode;

typedef struct PoolChunk {
    void *mem;
    struct PoolChunk *next;
} PoolChunk;

typedef struct Pool_s {
    alignas(POOL_CACHELINE_SIZE) size_t block_size;
    size_t block_count;
    ListNode *freelist;
    PoolChunk *chunks;
} Pool;

POOL_ALLOC_DEF Pool *pool_create(size_t num_blocks, size_t block_size);

POOL_ALLOC_DEF void pool_destroy(Pool *pool);

POOL_ALLOC_DEF void *pool_alloc(Pool *pool);

POOL_ALLOC_DEF void pool_free(Pool *pool, void *ptr);

POOL_ALLOC_DEF int pool_expand(Pool *pool, size_t extra_blocks);

#ifdef __cplusplus
}
#endif
#endif  // POOL_ALLOC_H_

#ifdef POOL_ALLOC_IMPLEMENTATION

static size_t pool_align_up(size_t n, size_t align) {
    return (n + align - 1) & ~(align - 1);
}

static int pool_add_chunk(Pool *pool, size_t num_blocks) {
    if (num_blocks == 0) return 0;

    if (pool->block_size != 0 && num_blocks > SIZE_MAX / pool->block_size)
        return 0;

    size_t total = num_blocks * pool->block_size;

    void *raw = POOL_ALLOC_BACKEND_MALLOC(total);
    if (!raw) return 0;

    PoolChunk *chunk = POOL_ALLOC_BACKEND_MALLOC(sizeof(PoolChunk));
    if (!chunk) {
        POOL_ALLOC_BACKEND_FREE(raw);
        return 0;
    }

    chunk->mem = raw;
    chunk->next = pool->chunks;
    pool->chunks = chunk;

    uint8_t *p = (uint8_t *)raw;

    for (size_t i = 0; i < num_blocks; ++i) {
        ListNode *node = (ListNode *)(p + i * pool->block_size);
        node->next = pool->freelist;
        pool->freelist = node;
    }

    pool->block_count += num_blocks;
    return 1;
}

POOL_ALLOC_DEF Pool *pool_create(size_t num_blocks, size_t block_size) {
    assert(num_blocks != 0);
    assert(block_size >= sizeof(ListNode));

    block_size = pool_align_up(block_size, POOL_CACHELINE_SIZE);

    Pool *pool = POOL_ALLOC_BACKEND_MALLOC(sizeof(Pool));
    if (!pool) return NULL;

    pool->block_size = block_size;
    pool->block_count = 0;
    pool->freelist = NULL;
    pool->chunks = NULL;

    if (!pool_add_chunk(pool, num_blocks)) {
        POOL_ALLOC_BACKEND_FREE(pool);
        return NULL;
    }

    return pool;
}

POOL_ALLOC_DEF void pool_destroy(Pool *pool) {
    if (!pool) return;

    PoolChunk *chunk = pool->chunks;
    while (chunk) {
        PoolChunk *next = chunk->next;
        POOL_ALLOC_BACKEND_FREE(chunk->mem);
        POOL_ALLOC_BACKEND_FREE(chunk);
        chunk = next;
    }

    POOL_ALLOC_BACKEND_FREE(pool);
}

POOL_ALLOC_DEF void *pool_alloc(Pool *pool) {
    if (!pool || !pool->freelist) return NULL;

    ListNode *node = pool->freelist;
    pool->freelist = node->next;
    return node;
}

POOL_ALLOC_DEF void pool_free(Pool *pool, void *ptr) {
    if (!pool || !ptr) return;

    ListNode *node = (ListNode *)ptr;
    node->next = pool->freelist;
    pool->freelist = node;
}

POOL_ALLOC_DEF int pool_expand(Pool *pool, size_t extra_blocks) {
    if (!pool) return 0;
    return pool_add_chunk(pool, extra_blocks);
}

#endif  // POOL_ALLOC_IMPLEMENTATION
