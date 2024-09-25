/*
 * Parser public interface. Returns a hash table with all of the symbols
 * in it.
 */
#ifndef _PARSE_H_
#define _PARSE_H_

#include "hash.h"

hash_table_t* load_config(const char* fname);

#endif /* _PARSE_H_ */
