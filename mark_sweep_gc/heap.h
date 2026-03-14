#ifndef HEAP_H_
#define HEAP_H_

#include <stddef.h>
#include <stdint.h>

int heap_init_stack_bounds(void);

#ifdef MEM_DEBUG
void *heap_alloc_dbg(size_t size, const char *name);
void heap_free_dbg(void *ptr, const char *name);
#define heap_alloc(size, name) heap_alloc_dbg((size), (name))
#define heap_free(ptr, name) heap_free_dbg((ptr), (name))
#else
void *heap_alloc_rel(size_t size);
void heap_free_rel(void *ptr);
#define heap_alloc(size, name) heap_alloc_rel((size))
#define heap_free(ptr, name) heap_free_rel((ptr))
#endif

void heap_info(void);

void heap_collect(void);

extern uintptr_t stack_low;
extern uintptr_t stack_high;

#endif  // HEAP_H_
