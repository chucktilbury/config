/*
 * Hash table implementation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "memory.h"
#include "hash.h"

#define MAX_HASH 5

/*
 * Don't barf if one of the strings is NULL and if they both are NULL, then 
 * compare equal.
 */
static inline int comp_str(const char* s1, const char* s2) {
    
    if(s1 != NULL && s2 != NULL)
        return strcmp(s1, s2);
    else if(s1 == NULL && s2 != NULL)
        return -1;
    else if(s1 != NULL && s2 == NULL)
        return 1;
    
    return 0;
}

/*
 * Generate the hash value for the string.
 */
static inline size_t create_hash(const char* key) {

#if 0
    size_t hash = 2166136261u;
    int slen = strlen(key);

    for(int i = 0; i < slen; i++) {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }
#else
    size_t hash = 0;
    size_t len = strlen(key);
    
    for(size_t i = 0; i < len; i++) {
        hash += key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

#endif

    return hash;
}

/*
 * Free the memory associated with a hah table entry.
 */
static void destroy_entry(hash_entry_t* entry) {
    
    if(entry != NULL) {
        if(entry->key != NULL)
            _FREE(entry->key);
        if(entry->data != NULL)
            _FREE(entry->data);
        _FREE(entry);
    }
}

/*
 * Find a table entry. Return NULL if the key is not found.
 */
static hash_entry_t* find_entry(hash_table_t* tab, const char* key) {

    size_t slot = create_hash(key) & (tab->cap - 1);
    
    hash_entry_t* crnt = tab->table[slot];
    while(crnt != NULL) {
        if(!comp_str(key, crnt->key)) 
            return crnt;
        else
            crnt = crnt->next;
    }
    
    return NULL;
}

/*
 * Remove an entry from the table. Silently fail if the entry is not found.
 */
static void remove_entry(hash_table_t* tab, const char* key) {
    
    hash_entry_t* entry = find_entry(tab, key);
    if(entry != NULL) {
        _FREE(entry->key);
        entry->key = NULL;
    }
}

/*
 * Add an entry to the table, where the entry has already been created.
 */
static void add_entry(hash_table_t* tab, hash_entry_t* entry) {

    size_t slot = create_hash(entry->key) & (tab->cap - 1);
    hash_entry_t* crnt;
    
    if(tab->table[slot] != NULL) {
        if(!comp_str(entry->key, tab->table[slot]->key)) {
            crnt = tab->table[slot];
            _FREE(tab->table[slot]->key);
            tab->table[slot]->key = NULL;
            while(crnt->next != NULL)
                crnt = crnt->next;
            crnt->next = entry;
            tab->len--;
            //fprintf(stderr, "ERROR: Cannot add duplicate hash table key: %s\n", entry->key);
            //exit(1);
        } 
        else {
            crnt = tab->table[slot];
            while(crnt->next != NULL) {
                if(!comp_str(entry->key, crnt->key)) {
                    _FREE(tab->table[slot]->key);
                    tab->table[slot]->key = NULL;
                    tab->len--;
                    //fprintf(stderr, "ERROR: Cannot add duplicate hash table key: %s\n", entry->key);
                    //exit(1);
                }
                crnt = crnt->next;
            }
            crnt->next = entry;
        }
    }
    else {
        tab->table[slot] = entry;
    }
}

/*
 * Grow the hash table and re-add all of the elements, but only if the number
 * of duplicate hash values is greater than MAX_HASH.
 */
static void rehash(hash_table_t* tab) {

    if(tab->len + MAX_HASH > tab->cap) {
        hash_entry_t* crnt;
        hash_entry_t* next;
        
        size_t old_cap = tab->cap;
        hash_entry_t** old_table = tab->table;
        
        tab->cap <<= 1;
        tab->table = _ALLOC_ARRAY(hash_entry_t*, tab->cap);
        
        for(size_t i = 0; i < old_cap; i++) {
            if(old_table[i] != NULL) {
                crnt = old_table[i];
                while(crnt != NULL) {
                    next = crnt->next;
                    crnt->next = NULL;
                    if(crnt->key != NULL)
                        add_entry(tab, crnt);
                    else
                        destroy_entry(crnt);
                    crnt = next;
                }
            }
        }
    }
}

/*
 * Allocate memory for the hash table entry.
 */
static hash_entry_t* create_entry(const char* key, const char* val) {
    
    hash_entry_t* ptr = _ALLOC_DS(hash_entry_t);
    ptr->key = _DUP_STR(key);
    ptr->data = _DUP_STR(val);
    ptr->next = NULL;
    
    return ptr;
}

/*
 * Allocate memory for the hash table.
 */
hash_table_t* create_hash_table(void) {
    
    hash_table_t* tab = _ALLOC_DS(hash_table_t);
    tab->len = 0;
    tab->cap = 1 << 3;
    tab->table = _ALLOC_ARRAY(hash_entry_t*, tab->cap);
    
    return tab;
}

/*
 * Free all of the memory associatated with the hash table.
 */
void destroy_hash_table(hash_table_t* tab) {

    hash_entry_t* crnt;
    hash_entry_t* next;
    for(size_t i = 0; i < tab->cap; i++) {
        if(tab->table[i] != NULL) {
            crnt = tab->table[i];
            while(crnt != NULL) {
                next = crnt->next;
                destroy_entry(crnt);
                crnt = next;
            }
        }
    }
}

/*
 * Add an entry to the hash table. Cannot add a duplicate entry. Note that 
 * going forward, a duplicate entry should probably replace the existing 
 * one.
 */
void add_table_entry(hash_table_t* tab, const char* key, const char* val) {
        
    rehash(tab);

    add_entry(tab, create_entry(key, val));
    tab->len++;
}

/*
 * Find a hash table entry and return the string associated with it. If the 
 * entry is not found, then return NULL.
 */
const char* find_table_entry(hash_table_t* tab, const char* key) {
    
    hash_entry_t* entry = find_entry(tab, key);
    
    if(entry != NULL)
        return entry->data;
    else
        return NULL;
}

/*
 * Remove a table entry by deleting the key. The actual memory will be freed 
 * when the table is rehashed.
 */
void remove_table_entry(hash_table_t* tab, const char* key) {
    
    remove_entry(tab, key);
}

/*
 * Dump the hash table to stdout for debugging.
 */
void dump_hash_table(hash_table_t* tab) {
    
    printf("\ntable cap = %lu\n", tab->cap);
    printf("table len = %lu\n", tab->len);
    printf("---------------------\n");
    
    hash_entry_t* crnt;
    int count = 1;
    int depth = 0;
    for(size_t i = 0; i < tab->cap; i++) {
        if(tab->table[i] != NULL) {
            crnt = tab->table[i];
            while(crnt != NULL) {
                printf("%3d. slot: %lu\t%s:\t%s\t(%d)\n", 
                        count++, i, crnt->key, crnt->data, depth++);
                crnt = crnt->next;
            }
        }
        depth = 0;
    }
    printf("\n");
}

/*
 * Below here is a simple sanity check for the hash table implementation.
 * 
 * Make string:
 * gcc -Wall -Wextra -Wpedantic -g -o t hash.c memory.c -DTEST_HASH_TABLE
 */
#ifdef TEST_HASH_TABLE

const char* keys[] = {
    "key0",
    "key1",
    "key2",
    "key3",
    "key4",
    "key5",
    "key6",
    "key7",
    "key8",
    "key9",
    "key10",
    "key11",
    "key12",
    "key13",
    "key14",
    "key15",
    "key16",
    "key17",
    "key18",
    "key19",
    "key20",
    "key21",
    "key22",
    "key23",
    "key24",
    "key25",
    "key26",
    "key27",
    "key28",
    "key29",
    NULL
};

int main(void) {
    
    hash_table_t* tab = create_hash_table();
    char buf[64];
    
    for(int i = 0; i < 5; i++) {
        sprintf(buf, "just a generic string number %d", i);
        add_table_entry(tab, keys[i], buf);
    }
        
    dump_hash_table(tab);
    
    char* key = "key17";
    printf("find: %s = %s\n", key, find_table_entry(tab, key));

    key = "key0";
    printf("find: %s = %s\n", key, find_table_entry(tab, key));

    key = "key29";
    printf("find: %s = %s\n", key, find_table_entry(tab, key));
    
    key = "key7";
    printf("find: %s = %s\n", key, find_table_entry(tab, key));

    key = "key20";
    printf("find: %s = %s\n", key, find_table_entry(tab, key));

    for(int i = 5; i < 14; i++) {
        sprintf(buf, "just a generic string number %d", i);
        add_table_entry(tab, keys[i], buf);
    }
    
    dump_hash_table(tab);

    for(int i = 7; i < 11; i++) {
        sprintf(buf, "replacement string number %d", i);
        add_table_entry(tab, keys[i], buf);
    }
    
    dump_hash_table(tab);

    for(int i = 14; i < 17; i++) {
        sprintf(buf, "just a generic string number %d", i);
        add_table_entry(tab, keys[i], buf);
    }
    
    dump_hash_table(tab);
    
    key = "key9";
    printf("find: %s = %s\n", key, find_table_entry(tab, key));
    
    key = "key7";
    printf("find: %s = %s\n", key, find_table_entry(tab, key));

    key = "key8";
    printf("find: %s = %s\n", key, find_table_entry(tab, key));

    for(int i = 17; keys[i] != NULL; i++) {
        sprintf(buf, "just a generic string number %d", i);
        add_table_entry(tab, keys[i], buf);
    }
    
    dump_hash_table(tab);

    destroy_hash_table(tab);
}

#endif
