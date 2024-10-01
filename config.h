/*
 * Config public interface.
 */
#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "hash.h"
#include "str.h"

typedef enum {
    CFG_ENV = 0x01,
    CFG_FILE = 0x02,
    CFG_CMD = 0x04,
    CFG_LIST = 0x80,
} config_entry_type_t;

typedef struct _config_entry_t_ {
    const char* name;
    config_entry_type_t type;
    string_t* raw;
    string_list_t* values;
} config_entry_t;

typedef struct _config_t_ {
    const char* pname;
    const char* name;
    const char* pream;
    const char* version;
    hash_table_t* vars;
    struct _cmdline_t_* cmdline;
} config_t;

config_t* init_configuration(const char* name,
                const char* preamble,
                const char* vers);

void load_configuration(config_t* cfg, int argc, char** argv, char** envp);

void add_config(config_t* cfg, const char* name, string_t* str, config_entry_type_t type);
string_t* get_config(const char* name);

#endif /* _CONFIG_H_ */
