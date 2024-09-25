/*
 * Simple memory wrapper to handle errors.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"

void* mem_alloc(size_t size) {
    
    void* ptr = malloc(size);
    if(ptr == NULL) {
        fprintf(stderr, "ERROR: Cannot allocate %lu bytes\n", size);
        exit(1);
    }
    
    memset(ptr, 0, size);
    return ptr;
}

void* mem_realloc(void* ptr, size_t size) {

    void* nptr = realloc(ptr, size);
    if(nptr == NULL) {
        fprintf(stderr, "ERROR: Cannot re-allocate %lu bytes\n", size);
        exit(1);
    }
    
    return nptr;
}

void* mem_dup(void* ptr, size_t size) {

    void* nptr = malloc(size);
    if(nptr == NULL) {
        fprintf(stderr, "ERROR: Cannot duplicate %lu bytes\n", size);
        exit(1);
    }

    memmove(nptr, ptr, size);
    return nptr;
}

const char* mem_dup_str(const char* ptr) {
    
    return (const char*)mem_dup((void*)ptr, strlen(ptr) + 1);
}

void mem_free(void* ptr) {
    
    //printf("ptr: %p\n", ptr);
    if(ptr != NULL)
        free(ptr);
}
