/*
 * Strings implementation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "memory.h"
#include "str.h"

/*
 * Create a new string with an optional initializer.
 */
string_t* create_string(const char* str) {

    string_t* ptr = _ALLOC_DS(string_t);
    ptr->cap = 1 << 3;
    ptr->len = 0;
    ptr->buf = _ALLOC_ARRAY(char, ptr->cap);

    if(str != NULL)
        append_string_str(ptr, str);

    return ptr;
}

/*
 * Free the memory associated with the string.
 */
void destroy_string(string_t* str) {

    if(str != NULL) {
        if(str->buf != NULL)
            _FREE(str->buf);
        _FREE(str);
    }
}

/*
 * Append a single character to the string.
 */
void append_string_char(string_t* ptr, int ch) {

    if(ptr->len+1 > ptr->cap) {
        ptr->cap <<= 1;
        ptr->buf = _REALLOC_ARRAY(ptr->buf, char, ptr->cap);
    }

    ptr->buf[ptr->len] = ch;
    ptr->len++;
    ptr->buf[ptr->len] = '\0';
}

/*
 * Append a native string to the string.
 */
void append_string_str(string_t* ptr, const char* str) {

    int len = strlen(str);
    if(ptr->len+len+1 > ptr->cap) {
        while(ptr->len+len+1 > ptr->cap)
            ptr->cap <<= 1;
        ptr->buf = _REALLOC_ARRAY(ptr->buf, char, ptr->cap);
    }

    memcpy(&ptr->buf[ptr->len], str, len+1);
    ptr->len += len;
}

/*
 * Append a string_t to the string.
 */
void append_string_string(string_t* ptr, string_t* str) {

    append_string_str(ptr, str->buf);
}

/*
 * Clear out the string, but do not change the length. Used when a scratch
 * buffer is needed.
 */
void clear_string(string_t* str) {

    str->len = 0;
    str->buf[0] = '\0';
}

/*
 * Return a native string from the string_t.
 */
const char* raw_string(string_t* str) {

    return str->buf;
}

/*
 * Strip the white from the beginning and the end of the string_t.
 */
void strip_string(string_t* str) {

    int i;
    for(i = str->len-1; i > 0; i--) {
        if(isspace(str->buf[i]))
            str->buf[i] = '\0';
        else
            break;
    }

    for(i = 0; isspace(str->buf[i]); i++) {}
    if(i > 0) {
        int len = strlen(str->buf);
        if(i < len)
            memmove(&str->buf[0], &str->buf[i], strlen(&str->buf[i])+1);
        else
            clear_string(str);
    }

    str->len = strlen(str->buf);
}

string_t* copy_string(string_t* str) {

    return create_string(str->buf);
}

int comp_string(string_t* s1, string_t*s2) {

    return strcmp(raw_string(s1), raw_string(s2));
}
