/*
 * Scanner public interface.
 */
#ifndef _SCAN_FILE_H_
#define _SCAN_FILE_H_

#include "str.h"

typedef enum {
    TOK_NO_TOKEN,
    TOK_NAME,       // [a-zA-Z_][a-zA-Z0-9_]*
    TOK_VALUE,      // all text up to the newline
    TOK_EQUAL,      // the '=' character
    TOK_OCBRACE,    // the '{' character
    TOK_CCBRACE,    // the '}' character
    TOK_QSTRG,      // anything between a pair of \" or \'
    TOK_END_OF_FILE, // end of input
} token_type_t;

typedef struct _token_t_ {
    string_t* str;
    token_type_t type;
} token_t;

void init_scanner(const char* fname);
token_t* get_token(void);
token_t* consume_token(void);
int get_line_no(void);
int get_col_no(void);
const char* get_fname(void);

#endif /* _SCAN_FILE_H_ */
