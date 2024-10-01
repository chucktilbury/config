/*
 * Implement the public interface to the configuration functionality.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <assert.h>

#include "parse_file.h"
#include "memory.h"
#include "config.h"

config_t* init_configuration(const char* name, const char* pre, const char* vers) {

    config_t* cfg = _ALLOC_DS(config_t);
    cfg->vars = create_hash_table();

    init_cmdline(cfg, name, pre, vers);

    return cfg;
}

void load_configuration(config_t* cfg, int argc, char** argv, char** envp) {

    cfg->pname = _DUP_STR(argv[0]);

    load_config_file(cfg);

    // load the environment into the table
    for(int i = 0; envp[i] != NULL; i++) {
        char* name = envp[i];
        char* val = strchr(name, '=');
        if(val != NULL) {
            *val = '\0';
            val++;
            add_table_entry(cfg->vars, name, create_string(val));
        }
    }

    parse_cmdline(cfg, argc, argv);
}

string_t* get_config_string(const char* name) {

    assert(name != NULL);
}

const char* get_config_str(const char* name) {

    assert(name != NULL);
}

unsigned long get_config_unsigned(const char* name) {

    assert(name != NULL);
}

long get_config_integer(const char* name) {

    assert(name != NULL);
}

double get_config_float(const char* name) {

    assert(name != NULL);
}
