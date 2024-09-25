/*
 * Config file parser implementation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>

#include "scan.h"
#include "str.h"
#include "parse.h"
#include "memory.h"

#define FATAL(f, ...) do { \
        fprintf(stderr, "ERROR: "); \
        fprintf(stderr, f __VA_OPT__(,) __VA_ARGS__); \
        fprintf(stderr, "\n"); \
        exit(1); \
    } while(0)

#define ERROR(f, ...) do { \
        fprintf(stderr, "ERROR: %s: %d: %d: ", get_fname(), get_line_no(), get_col_no()); \
        fprintf(stderr, f __VA_OPT__(,) __VA_ARGS__); \
        fprintf(stderr, "\n"); \
    } while(0)

#define EXPECTED(s) do { \
        fprintf(stderr, "ERROR: %s: %d: %d: ", get_fname(), get_line_no(), get_col_no()); \
        fprintf(stderr, "Expected %s but got a '%s'\n", (s), get_token()->str->buf); \
    } while(0)

#ifdef USE_TRACE
#define TRACE fprintf(stderr, "state: %d %s\n", state, get_token()->str->buf)
#else
#define TRACE
#endif

typedef struct {
    string_t** list;
    int len; 
    int cap;
} context_t;

static context_t* context;

void push_context(string_t* name) {
    
    if(context->len+1 > context->cap) {
        context->cap <<= 1;
        context->list = _REALLOC_ARRAY(context->list, string_t*, context->cap);
    }
    
    context->list[context->len] = create_string(name->buf);
    context->len++;
}

void pop_context(void) {
    
    if(context->len) {
        context->len--;
        destroy_string(context->list[context->len]);
        context->list[context->len] = NULL;
    }
    else {
        ERROR("imbalanced '{}'");
        FATAL("Context stack under-run");
    }
}

const char* context_name(string_t* name) {
    
    string_t* str = create_string(NULL);
    
    if(context->len) {
        append_string_string(str, context->list[0]);
        for(int i = 1; i < context->len; i++) {
            append_string_char(str, '.');
            append_string_string(str, context->list[i]);
        }
        append_string_char(str, '.');
    }
    
    append_string_string(str, name);
    
    return str->buf;
}


/*
 * Load the whole configuration file and deliver the result.
 */
hash_table_t* load_config(const char* fname) {
    
    init_scanner(fname);
    hash_table_t* table = create_hash_table();
    context = _ALLOC_DS(context_t);
    context->cap = 1 << 3;
    context->len = 0;
    context->list = _ALLOC_ARRAY(string_t*, context->cap);
    
    int finished = 0;
    int state = 0;
    
    string_t* name = create_string(NULL);

    while(!finished) {
        token_t* tok = get_token();
        switch(state) {
            
            case 0:
                TRACE;
                // expecting a name or an error
                if(tok->type == TOK_NAME) {
                    append_string_string(name, tok->str);
                    consume_token();
                    state = 1;
                }
                else if(tok->type == TOK_END_OF_FILE) {
                    finished++;
                }
                else {
                    EXPECTED("a NAME");
                    exit(1);
                }
                break;
                
            case 1:
                TRACE;
                // expecting a value or a '{'
                if(tok->type == TOK_VALUE) {
                    add_table_entry(table, context_name(name), tok->str->buf);
                    clear_string(name);
                    consume_token();
                    state = 2;
                }
                else if(tok->type == TOK_OCBRACE) {
                    push_context(name);
                    clear_string(name);
                    consume_token();
                    state = 2;
                }
                else {
                    EXPECTED("a VALUE or a '{'");
                    exit(1);
                }
                break;
                
            case 2:
                TRACE;
                // need a NAME or a '}'
                if(tok->type == TOK_NAME) {
                    append_string_string(name, tok->str);
                    consume_token();
                    state = 1;
                }
                else if(tok->type == TOK_CCBRACE) {
                    pop_context();
                    consume_token();
                    //state = 0;
                }
                else if(tok->type == TOK_END_OF_FILE) {
                    finished++;
                }
                else {
                    EXPECTED("a NAME or a '}'");
                    exit(1);
                }
                break;
        }
    }
    
    if(context->len != 0) {
        ERROR("Unexpected end of file, imbalanced '{}'");
        exit(1);
    }
    
    _FREE(context->list);
    _FREE(context);
    
    return table;
}
