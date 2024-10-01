/*
 * Simple memory wrapper to simplify error handling.
 */
#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <stddef.h> // for size_t

#define _ALLOC(s) mem_alloc(s)
#define _ALLOC_DS(t) (t*)mem_alloc(sizeof(t))
#define _ALLOC_ARRAY(t, n) (t*)mem_alloc(sizeof(t)*(n))
#define _REALLOC(p, s) mem_realloc((void*)(p), (s))
#define _REALLOC_ARRAY(p, t, n) (t*)mem_realloc((void*)(p), (n)*(sizeof(t)))
#define _DUP_MEM(p, s) mem_dup((void*)(p), (s))
#define _DUP_STR(s) mem_dup_str((const char*)(s))
#define _FREE(p) mem_free((void*)(p))

void* mem_alloc(size_t size);
void* mem_realloc(void* ptr, size_t size);
void* mem_dup(void* ptr, size_t size);
char* mem_dup_str(const char* ptr);
void mem_free(void* ptr);

#endif /* _MEMORY_H_ */
