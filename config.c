/*
 * 
 */

#include "parse.h"

int main(void) {
    
    hash_table_t* table = load_config("test.cfg");
    dump_hash_table(table);
    return 0;
}
