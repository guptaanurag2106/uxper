#define _GNU_SOURCE
#include "heap.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UTILS_IMPLEMENTATION
#include "../utils/utils.h"

typedef enum { ALLOCATED, FREED, COLLECTED, MARKED } AllocState;

static uintptr_t heap_min = UINTPTR_MAX;
static uintptr_t heap_max = 0;
uintptr_t stack_low = 0;
uintptr_t stack_high = 0;
static int stack_grows_downward = 1;

static int detect_stack_growth_impl(uintptr_t parent_addr) {
    volatile uintptr_t probe = 0;
    return (uintptr_t)&probe < parent_addr;
}

static int detect_stack_growth(void) {
    volatile uintptr_t probe = 0;
    return detect_stack_growth_impl((uintptr_t)&probe);
}

int heap_init_stack_bounds(void) {
    pthread_attr_t attr;
    void *stack_addr = NULL;
    size_t stack_size = 0;

    int rc = pthread_getattr_np(pthread_self(), &attr);
    if (rc != 0) {
        return -1;
    }

    rc = pthread_attr_getstack(&attr, &stack_addr, &stack_size);
    pthread_attr_destroy(&attr);
    if (rc != 0) {
        return -1;
    }

    stack_low = (uintptr_t)stack_addr;
    stack_high = (uintptr_t)stack_addr + stack_size;
    stack_grows_downward = detect_stack_growth();
    return 0;
}

const char *alloc_state_to_string(AllocState state) {
    switch (state) {
        case ALLOCATED:
            return "Allocated";
        case FREED:
            return "Freed";
        case COLLECTED:
            return "Collected";
        case MARKED:
            return "Marked";
        default:
            UNREACHABLE("unreachable");
    }
}

typedef struct {
    void *ptr;
    size_t size;
    AllocState state;
    const char *name;
} Record;

Vector(Record, Records);
static Records records = {0};

#ifdef MEM_DEBUG
void *heap_alloc_dbg(size_t size, const char *name) {
#else
void *heap_alloc_rel(size_t size) {
#endif
    if (size == 0) {
        size = 1;
    }

    void *res = malloc(size);
    if (res == NULL) {
        fprintf(stderr, "heap_alloc: malloc failed for %zu bytes\n", size);
        return NULL;
    }
    void *ptr = res;

#ifdef MEM_DEBUG
    if (name == NULL) {
        fprintf(stderr, "heap_alloc: name cannot be NULL in MEM_DEBUG\n");
        free(res);
        return NULL;
    }
    Record record = {ptr, size, ALLOCATED, name};
#else
    Record record = {ptr, size, ALLOCATED, NULL};
#endif

    vec_push(&records, record);
    heap_min = MIN(heap_min, (uintptr_t)ptr);
    heap_max = MAX(heap_max, (uintptr_t)ptr);

    return ptr;
}

#ifdef MEM_DEBUG
void heap_free_dbg(void *ptr, const char *name) {
#else
void heap_free_rel(void *ptr) {
#endif
    if (ptr == NULL) {
        return;
    }

    Record *found = NULL;
    vec_foreach(&records, rec) {
        if (rec->ptr == ptr) {
            if (rec->state == ALLOCATED || rec->state == MARKED) {
                found = rec;
                break;
            }
            if (found == NULL) {
                found = rec;
            }
        }
    }

    if (found == NULL) {
        fprintf(stderr,
                "heap_free: pointer %p was not allocated by heap_alloc\n", ptr);
        exit(1);
    }

    if (found->state == FREED || found->state == COLLECTED) {
        fprintf(stderr, "heap_free: double free detected for %p\n", ptr);
        exit(1);
    }

#ifdef MEM_DEBUG
    if (name == NULL) {
        fprintf(stderr, "heap_free: name cannot be NULL in MEM_DEBUG\n");
        exit(1);
    }
    if (found->name == NULL || strcmp(found->name, name) != 0) {
        fprintf(stderr, "heap_free: expected %s got %s\n",
                found->name ? found->name : "(null)", name);
        exit(1);
    }
#endif

    found->state = FREED;
    free(found->ptr);
}

void heap_info(void) {
#ifndef MEM_DEBUG
    return;
#endif
    vec_foreach(&records, rec) {
        printf("%s: %s at %p of size %zu\n", alloc_state_to_string(rec->state),
               rec->name ? rec->name : "(null)", rec->ptr, rec->size);
    }
}

static void recursive_mark(uintptr_t start, uintptr_t end_exclusive) {
    const uintptr_t word = sizeof(uintptr_t);
    if (end_exclusive <= start || (end_exclusive - start) < word) {
        return;
    }

    uintptr_t aligned_start = (start + (word - 1)) & ~(uintptr_t)(word - 1);
    if (aligned_start >= end_exclusive ||
        (end_exclusive - aligned_start) < word) {
        return;
    }

    for (uintptr_t addr = aligned_start; addr <= end_exclusive - word;
         addr += word) {
        uintptr_t value = *(uintptr_t *)addr;
        if (value <= heap_max && value >= heap_min) {
            vec_foreach(&records, rec) {
                if (rec->state == ALLOCATED && (uintptr_t)rec->ptr == value) {
                    rec->state = MARKED;
                    uintptr_t child_start = (uintptr_t)rec->ptr;
                    uintptr_t child_end_exclusive = child_start + rec->size;
                    if (child_end_exclusive > child_start) {
                        recursive_mark(child_start, child_end_exclusive);
                    }
                    break;
                }
            }
        }
    }
}

void heap_collect(void) {
    volatile uintptr_t probe = 0;
    uintptr_t *sp = (uintptr_t *)&probe;
    if (stack_low == 0 || stack_high == 0) {
        fprintf(stderr, "heap_collect: stack bounds not initialized\n");
        return;
    }

    uintptr_t start, end;
    if (stack_grows_downward) {
        start = (uintptr_t)sp;
        end = stack_high;
    } else {
        start = stack_low;
        end = (uintptr_t)sp;
    }

    /* printf("start: 0x%lx, end: 0x%lx; heap_min: 0x%lx, heap_max: 0x%lx\n", */
    /*        start, end, heap_min, heap_max); */
    recursive_mark(start, end);

    vec_foreach(&records, rec) {
        if (rec->state == ALLOCATED) {
            heap_free(rec->ptr, rec->name);
            rec->state = COLLECTED;
        }
        if (rec->state == MARKED) {
            rec->state = ALLOCATED;
        }
    }
}
