/*
 * Hash table public interface.
 */
#ifndef _HASH_H_
#define _HASH_H_

#include <stddef.h>
#include "strlist.h"

typedef struct _hash_entry_ {
    const char* key;
    void* val;
    struct _hash_entry_* next;
} hash_entry_t;

typedef struct _hash_table_t_ {
    struct _hash_entry_** table;
    size_t len;
    size_t cap;
} hash_table_t;

hash_table_t* create_hash_table(void);
void destroy_hash_table(hash_table_t* tab);
void add_table_entry(hash_table_t* tab, const char* key, void* val);
void* find_table_entry(hash_table_t* tab, const char* key);
void dump_hash_table(hash_table_t* tab, void (*vdump)(void*));

#endif /* _HASH_H_ */
