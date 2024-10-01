/*
 * Parser public interface. Returns a hash table with all of the symbols
 * in it.
 */
#ifndef _PARSE_FILE_H_
#define _PARSE_FILE_H_

#include "hash.h"
#include "config.h"

const char* find_config_file(config_t* cfg);
void load_config_file(config_t* cfg);

#endif /* _PARSE_FILE_H_ */
