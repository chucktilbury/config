/*
 * String list public interface.
 */
#ifndef _STRING_LIST_H_
#define _STRING_LIST_H_

#include "str.h"

typedef struct _string_list_t_ {
    string_t** list;
    int cap;
    int len;
} string_list_t;

string_list_t* create_string_list(void);
string_list_t* convert_string(string_t* str);
void destroy_string_list(string_list_t* lst);
void append_string_list(string_list_t* lst, string_t* str);
string_t* iter_string_list(string_list_t* lst, int* mark);

#endif /* _STRING_LIST_H_ */

