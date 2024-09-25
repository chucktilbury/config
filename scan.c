/*
 * Implement a simple scanner for the input file.
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>



#include "scan.h"
#include "memory.h"

typedef struct {
    const char* fname;
    FILE* fp;
    int line;
    int col;
    token_t tok;
    int ch;
} scanner_t;

static scanner_t* scanner;

static int get_char(void) {
    
    return scanner->ch;
}

static int consume_char(void) {
    
    if(feof(scanner->fp)) 
        return scanner->ch;
    else if(scanner->ch == '\n') {
        scanner->line++;
        scanner->col = 1;
    }
    else
        scanner->col++;
        
    scanner->ch = fgetc(scanner->fp);
    if(scanner->ch == EOF)
    
        clear_string(scanner->tok.str);
    return scanner->ch;
}

static int is_name_stopper(int ch) {
    
    if(isalpha(ch) || ch == '_')
        return 0;
    else 
        return 1;
}

static int is_value_stopper(int ch) {
    
    if(ch == '{' || ch == '}' || ch == '=' || ch == ';' || ch == '\n' || ch == EOF)
        return 1;
    else
        return 0;
}

static void consume_comment(void) {
    
    int ch;
    
    do {
        ch = consume_char();
    } while(ch != '\n' && ch != EOF);
}

static void scan_name(void) {

    int ch = get_char();
    
    while(!is_name_stopper(ch)) {
        append_string_char(scanner->tok.str, ch);
        ch = consume_char();
    }     
    
    scanner->tok.type = TOK_NAME;
}

static void scan_string(void) {
    
    int ch, ender = get_char();
    ch = consume_char();
    
    while(ch != ender && ch != EOF) {
        append_string_char(scanner->tok.str, ch);
        ch = consume_char();
    } 
    
    if(ch == EOF) {
        fprintf(stderr, "ERROR: %s: %d: %d: Unexpected end of file\n", 
                get_fname(), get_line_no(), get_col_no());
        exit(1);
    }
    else
        consume_char();

    //scanner->tok.type = TOK_QSTRG;
}

static void scan_value(void) {
    
    int ch = get_char();
    
    while(isspace(ch)) {
        ch = consume_char();
    }
    
    if(ch == EOF) {
        fprintf(stderr, "ERROR: %s: %d: %d: Expected a value but got EOF\n",
                    get_fname(), get_line_no(), get_col_no());
        exit(1);
    }
    else if(is_value_stopper(ch)) {
        fprintf(stderr, "ERROR: %s: %d: %d: Expected a value but got '%c'\n",
                    get_fname(), get_line_no(), get_col_no(), ch);
        exit(1);
    }        
    
    if(ch == '\'' || ch == '\"')
        scan_string();
    else {
        while(!is_value_stopper(ch)) { 
            append_string_char(scanner->tok.str, ch);
            ch = consume_char();
        }
        strip_string(scanner->tok.str);
    }
    
    scanner->tok.type = TOK_VALUE;
}

/*
 * Functions below this point are public interface. Ones above this point are
 * private implementation.
 */

/*
 * Initialize the scanner. This must be called before reading any characters.
 */
void init_scanner(const char* fname) {
    
    FILE* fp = fopen(fname, "r");
    if(fp == NULL) {
        fprintf(stderr, "ERROR: Unable to open input file %s: %s\n",
                fname, strerror(errno));
        exit(1);
    }
    
    scanner = _ALLOC_DS(scanner_t);
    scanner->fp = fp;
    scanner->fname = _DUP_STR(fname);
    scanner->line = 1;
    scanner->col = 1;
    scanner->tok.str = create_string(NULL);
    scanner->tok.type = TOK_NO_TOKEN;
    scanner->ch = fgetc(fp);
    
    consume_token();
}

/*
 * Dispose of the current token and get the next one. Return a pointer to it.
 */
token_t* consume_token(void) {
    
    clear_string(scanner->tok.str);
    int finished = 0;
    
    while(!finished) {
        int ch = get_char();
        switch(ch) {
            case EOF:
                scanner->tok.type = TOK_END_OF_FILE;
                finished++;
                break;
            case '{':
                append_string_char(scanner->tok.str, '{');
                scanner->tok.type = TOK_OCBRACE;
                consume_char();
                finished++;
                break;
            case '}':
                append_string_char(scanner->tok.str, '}');
                scanner->tok.type = TOK_CCBRACE;
                consume_char();
                finished++;
                break;
            case '=':
                consume_char();
                scan_value();
                finished++;
                break;
            case ';':
                consume_comment();
                break;
            case '\"':
            case '\'':
                scan_string();
                finished++;
                break;
            default:
                if(isspace(ch)) {
                    consume_char();
                    break;
                }
                else {
                    scan_name();
                    finished++;
                }
                break;
        }
    }
        
    return &scanner->tok;
}

/*
 * Return a pointer to the current token.
 */
token_t* get_token(void) {
    
    return &scanner->tok;
}

/*
 * Return the current line number.
 */
int get_line_no(void) {
    
    return scanner->line;
}

/*
 * Return the current column number.
 */
int get_col_no(void) {
    
    return scanner->col;
}

/*
 * Return the current file name.
 */
const char* get_fname(void) {
    
    return scanner->fname;
}

/*
 * Below here is code to test the scanner.
 */
#ifdef TEST_SCANNER

static const char* type_to_str(int type) {

    return
    (type == TOK_NO_TOKEN)? "TOK_NO_TOKEN":
    (type == TOK_NAME)? "TOK_NAME":
    (type == TOK_VALUE)? "TOK_VALUE":
    (type == TOK_EQUAL)? "TOK_EQUAL":
    (type == TOK_OCBRACE)? "TOK_OCBRACE":
    (type == TOK_CCBRACE)? "TOK_CCBRACE":
    (type == TOK_QSTRG)? "TOK_QSTRG":
    (type == TOK_END_OF_FILE)? "TOK_END_OF_FILE": "UNKNOWN";
}

static void dump_token(token_t* tok) {
    
    printf("token str: \"%s\" type: %s (%d) %d: %d\n", 
            tok->str->buf, type_to_str(tok->type), tok->type,
            get_line_no(), get_col_no());
}

int main(void) {
 
    init_scanner("test.cfg");
    token_t* tok = get_token();
    
    while(tok->type != TOK_END_OF_FILE) {
        dump_token(tok);
        tok = consume_token();
    } 
    dump_token(tok);
    
    return 0;
}

#endif
