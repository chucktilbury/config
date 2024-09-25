/*
 * Strings implementation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "memory.h"
#include "str.h"

string_t* create_string(const char* str) {
    
    string_t* ptr = _ALLOC_DS(string_t);
    ptr->cap = 1 << 3;
    ptr->len = 0;
    ptr->buf = _ALLOC_ARRAY(char, ptr->cap);
    
    if(str != NULL)
        append_string_str(ptr, str);
        
    return ptr;
}

void destroy_string(string_t* str) {
    
    if(str != NULL) {
        if(str->buf != NULL)
            _FREE(str->buf);
        _FREE(str);
    }
}

void append_string_char(string_t* ptr, int ch) {
    
    if(ptr->len+1 > ptr->cap) {
        ptr->cap <<= 1;
        ptr->buf = _REALLOC_ARRAY(ptr->buf, char, ptr->cap);
    }
    
    ptr->buf[ptr->len] = ch;
    ptr->len++;
    ptr->buf[ptr->len] = '\0';
}

void append_string_str(string_t* ptr, const char* str) {
    
    int len = strlen(str);
    if(ptr->len+len+1 > ptr->cap) {
        while(ptr->len+len+1 < ptr->cap)
            ptr->cap <<= 1;
        ptr->buf = _REALLOC_ARRAY(ptr->buf, char, ptr->cap);
    }
    
    memcpy(&ptr->buf[ptr->len], str, len+1);
    ptr->len += len;
}

void append_string_string(string_t* ptr, string_t* str) {
    
    append_string_str(ptr, str->buf);
}

void clear_string(string_t* str) {
    
    str->len = 0;
    str->buf[0] = '\0';
}

const char* raw_string(string_t* str) {
    
    return str->buf;
}

void strip_string(string_t* str) {
    
    for(int i = str->len-1; i > 0; i--) {
        if(isspace(str->buf[i])) {
            str->buf[i] = '\0';
            str->len--;
        }
        else 
            break;
    }
}

