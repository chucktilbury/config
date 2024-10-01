/*
 * Implement the string list functions.
 */
#include <string.h>
#include <stdio.h>

#include "memory.h"
#include "strlist.h"

string_list_t* create_string_list(void) {
    
    string_list_t* ptr = _ALLOC_DS(string_list_t);
    ptr->cap = 1 << 3;
    ptr->len = 0;
    ptr->list = _ALLOC_ARRAY(string_t*, ptr->cap);
    
    return ptr;
}

void destroy_string_list(string_list_t* lst) {
    
    if(lst != NULL) {
        int mark = 0;
        string_t* ptr;
        while(NULL != (ptr = iter_string_list(lst, &mark))) 
            destroy_string(ptr);
        _FREE(lst);
    }
}

void append_string_list(string_list_t* lst, string_t* str) {
    
    if(lst->len+1 > lst->cap) {
        lst->cap <<= 1;
        lst->list = _REALLOC_ARRAY(lst->list, string_t*, lst->cap);
    }
    
    lst->list[lst->len] = str;
    lst->len ++;
}

string_t* iter_string_list(string_list_t* lst, int* mark) {
    
    string_t* val = NULL;
    
    if(lst->len > *mark) {
        val = lst->list[*mark];
        *mark = *mark + 1;
    }
    
    return val;
}

/*
 * A list is divided by a ':' character. Unless the first character is a ':',
 * in which case, the item is a list of one item. Does not touch the original.
 * Uses strtok() to split the string.
 */
string_list_t* convert_string(string_t* str) {
    
    //fprintf(stderr, "HERHE!\n");
    string_list_t* lst = create_string_list();
    
    if(str->buf[0] == ':') {
        //fprintf(stderr, "flarp\n");
        append_string_list(lst, str);
        return lst;
    }
    
    char* buf = _DUP_STR(str->buf);
    
    for(char* token = strtok(buf, ":"); 
                token != NULL; token = strtok(NULL, ":")) {
        //fprintf(stderr, "%s\n", token);
        append_string_list(lst, create_string(token));            
    }
    
    _FREE(buf);
    
    //fprintf(stderr, "len: %d\n", lst->len);
    return lst;
}
