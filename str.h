/*
 * Strings public interface.
 */
#ifndef _STR_H_
#define _STR_H_

typedef struct {
    char* buf;
    int len;
    int cap;
} string_t;

string_t* create_string(const char* str);
void destroy_string(string_t* str);
void append_string_char(string_t* ptr, int ch);
void append_string_str(string_t* ptr, const char* str);
void append_string_string(string_t* ptr, string_t* str);
void clear_string(string_t* str);
const char* raw_string(string_t* str);
void strip_string(string_t* str);

#endif /* _STR_H_ */
