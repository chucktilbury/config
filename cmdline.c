/*
 * Command line implementation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "cmdline.h"
#include "memory.h"

#define EXPECTED(s) do { \
        fprintf(stderr, "CMDLINE: Expected %s but got '%c'\n", (s), get_char()); \
    } while(0)

#define ERROR(s, ...) do { \
        fprintf(stderr, "CMDLINE: "); \
        fprintf(stderr, s __VA_OPT__(,) __VA_ARGS__ ); \
        fprintf(stderr, "\n"); \
    } while(0)

/*********************************************
 * Private functions
 */
static const char* cmdstr = NULL;
static int cmdstr_idx = 0;

static int get_char(void) {

    return cmdstr[cmdstr_idx];
}

static int consume_char(void) {

    if(cmdstr[cmdstr_idx] != 0)
        cmdstr_idx++;

    return get_char();
}

static cmdline_entry_t* get_short_opt(config_t* cfg, int ch) {

    cmdline_entry_t* ptr = cfg->cmdline->first;
    while(ptr != NULL) {
        if(ptr->short_opt == ch)
            break;
        else
            ptr = ptr->next;
    }

    return ptr;
}

static cmdline_entry_t* get_long_opt(config_t* cfg, const char* str) {

    cmdline_entry_t* ptr = cfg->cmdline->first;
    while(ptr != NULL) {
        if(!strcmp(str, ptr->long_opt))
            break;
        else
            ptr = ptr->next;
    }

    return ptr;
}

static cmdline_entry_t* get_opt(config_t* cfg, const char* str) {

    cmdline_entry_t* ptr = cfg->cmdline->first;
    while(ptr != NULL) {
        if(!strcmp(str, ptr->name))
            break;
        else
            ptr = ptr->next;
    }

    return ptr;
}

static const char* parm_type_to_str(cmdline_type_t type) {

    return
    ((type & CMD_STR) && !((type & CMD_NUM) || (CMD_BOOL)))? "STR ":
    ((type & CMD_NUM) && !((type & CMD_STR) || (CMD_BOOL)))? "NUM ":
    ((type & CMD_BOOL) && !((type & CMD_NUM) || (CMD_STR)))? "BOOL ": "";
}

/*********************************************
 * APIs used by this software.
 */
void destroy_cmdline(cmdline_t* cmd) {

    if(cmd != NULL) {
        if(cmd->preamble != NULL)
            _FREE(cmd->preamble);
        if(cmd->vers != NULL)
            _FREE(cmd->vers);
        if(cmd->name != NULL)
            _FREE(cmd->name);
        if(cmd->first != NULL) {
            cmdline_entry_t* crnt;
            cmdline_entry_t* next;
            for(crnt = cmd->first; crnt != NULL; crnt = next) {
                next = crnt->next;
                if(crnt->long_opt != NULL)
                    _FREE(crnt->long_opt);
                if(crnt->name != NULL)
                    _FREE(crnt->name);
                if(crnt->help != NULL)
                    _FREE(crnt->help);
                if(crnt->def_val != NULL)
                    _FREE(crnt->def_val);
                _FREE(crnt);
            }
        }
        _FREE(cmd);
    }
}

/*
 * When this is entered, the first character of the value is the current
 * character. If the value starts with a single or a double quote, then it can
 * contain spaces. If it starts with any other character, then the first space
 * marks the end of it.
 */
static int parse_value(config_t* cfg, cmdline_entry_t* entry) {

    int finished = 0;
    int state = 0;

    int stopper;
    string_t* str = create_string(NULL);

    while(!finished) {
        int ch = get_char();
        switch(state) {
            case 0:
                if(ch == '\'' || ch == '\"') {
                    stopper = ch;
                    consume_char();
                    state = 1;
                }
                else if(isalnum(ch) || ispunct(ch))
                    state = 2;
                else {
                    EXPECTED("a command parameter value");
                    state = 200;
                }
                break;

            case 1:
                // read a string
                if(ch == stopper)
                    state = 100;
                if(ch == '\0') {
                    ERROR("unexpected end of line in quoted string");
                    state = 200;
                }
                else {
                    append_string_char(str, ch);
                    consume_char();
                }
                break;

            case 2:
                // read until a space is encountered
                if(ch == ' ' || ch == '\0')
                    state = 100;
                else {
                    append_string_char(str, ch);
                    consume_char();
                }
                break;

            case 100:
                // end of line is reached
                add_table_entry(cfg->vars, entry->name, str);
                finished++;
                break;

            case 200:
                finished++;
                break;

            default:
                fprintf(stderr, "FATAL ERROR: Invalid state in %s: %d\n",
                        __func__, state);
                exit(1);
        }
    }

    return state;
}

/*
 * When this is entered, the current char is the first short option. More than
 * short option can appear if they all are switches, if the short option is not
 * a switch and the following char is not a space or an '=' then an error is
 * published. This returns all short options have been read and verified.
 */
static int parse_short_option(config_t* cfg) {

    int finished = 0;
    int state = 0;

    cmdline_entry_t* ptr;
    int parm;

    while(!finished) {
        int ch = get_char();
        switch(state) {
            case 0:
                ptr = get_short_opt(cfg, ch);
                if(ptr != NULL) {
                    parm = ch;
                    ptr->type |= CMD_SEEN;
                    if(ptr->type & CMD_RARG)
                        state = 1;
                    else if(ptr->type & CMD_OARG)
                        state = 2;
                    // else if it's CMD_NARG, then just get the next one
                    consume_char();
                }
                else {
                    ERROR("Unknown short option: %c", ch);
                    state = 200;
                }
                break;

            case 1:
                ch = consume_char(); // consume the short arg
                if(ch == ' ' || ch == '=') {
                    consume_char();
                    state = 3;
                }
                else {
                    ERROR("A required %sparameter for -%c",
                                parm_type_to_str(ptr->type), parm);
                    state = 200;
                }
                break;

            case 2:
                ch = consume_char(); // consume the short arg
                if(ch == ' ' || ch == '=') {
                    consume_char();
                    state = 3;
                }
                else
                    state = 0;
                break;

            case 3:
                // consume the value and put it in the DS
                state = parse_value(cfg, ptr);
                break;

            case 200:
            case 100:
                // end of line is reached
                finished++;
                break;

            default:
                fprintf(stderr, "FATAL ERROR: Invalid state in %s: %d\n",
                        __func__, state);
                exit(1);
        }
    }
    return state;
}

/*
 * When this is entered, the current char is the first character of the long
 * name. This returns when the complete long option has been read and verified.
 */
static int parse_long_option(config_t* cfg) {

    int finished = 0;
    int state = 0;

    string_t* str = create_string(NULL);
    cmdline_entry_t* ptr;

    while(!finished) {
        int ch = get_char();
        switch(state) {
            case 0:
                // get the long option name
                if(isalnum(ch) || ispunct(ch)) {
                    append_string_char(str, ch);
                    consume_char();
                }
                else if(ch == '\0')
                    state = 100;
                else
                    state = 1;
                break;

            case 1:
                // set SEEN and handle options
                ptr = get_long_opt(cfg, str->buf);
                if(ptr != NULL) {
                    ptr->type |= CMD_SEEN;
                    if(ptr->type & CMD_RARG)
                        state = 2;
                    else if(ptr->type & CMD_OARG)
                        state = 3;
                    // else if it's CMD_NARG, then just get the next one
                }
                else {
                    ERROR("unknown long option: --%s", str->buf);
                    state = 200;
                }
                break;

            case 2:
                if(ch == ' ' || ch == '=') {
                    consume_char();
                    state = 4;
                }
                else {
                    EXPECTED("a ' ' or a '='");
                    state = 200;
                }
                break;

            case 3:
                if(ch == '=') {
                    consume_char();
                    state = 5;
                }
                else if(ch == ' ') {
                    consume_char();
                    state = 4;
                }
                break;

            case 4:
                if(ch == '-')
                    state = 100;
                else
                    state = 5;
                break;

            case 5:
                // consume the value and put it in the DS
                state = parse_value(cfg, ptr);
                break;

            case 100:
                // end of line is reached
                finished++;
                break;

            case 200:
                finished++;
                break;

            default:
                fprintf(stderr, "FATAL ERROR: Invalid state in %s: %d\n",
                        __func__, state);
                exit(1);
        }
    }
    return state;
}

/*
 * When this is entered, the current char is the first '-'. This returns when
 * option has been completed or an error has happened.
 */
static int parse_option(config_t* cfg) {

    int finished = 0;
    int state = 0;

    consume_char(); // consume the leading '-'

    while(!finished) {
        int ch = get_char();
        switch(state) {
            case 0:
                if(ch == '-') {
                    consume_char();
                    state = parse_long_option(cfg);
                }
                else
                    state = parse_short_option(cfg);
                break;

            case 100:
            case 200:
                // end of line is reached
                finished++;
                break;

            default:
                fprintf(stderr, "FATAL ERROR: Invalid state in %s: %d\n",
                        __func__, state);
                exit(1);
        }
    }
    return state;
}


static int parse_list_item(config_t* cfg) {

    int finished = 0;
    int state = 0;
    int ch;

    while(!finished) {
        ch = get_char();
        switch(state) {
            case 0:
                break;

            case 100:
                finished++;
                break;

            case 200:
                finished++;
                break;

            default:
                fprintf(stderr, "FATAL ERROR: Invalid state in %s: %d\n",
                        __func__, state);
                exit(1);
        }
    }
}

void parse_cmdline(config_t* cfg, int argc, char** argv) {

    string_t* str = create_string(NULL);

    for(int i = 1; i < argc; i++) {
        append_string_str(str, argv[i]);
        append_string_char(str, ' ');
    }
    cmdstr = _DUP_STR(str->buf);
    clear_string(str);

    printf("cmdline: '%s'\n", cmdstr);

    int finished = 0;
    int state = 0;

    while(!finished) {
        int ch = get_char();
        switch(state) {
            case 0:
                if(ch == '\0')
                    state = 100;
                else if(ch == '-')
                    state = parse_option(cfg);
                else
                    state = parse_list_item(cfg);
                break;

            case 100:
                // end of line is reached
                finished++;
                break;

            case 200:
                // error is encountered
                // all errors abort the program.
                exit(1);
                break;

            default:
                fprintf(stderr, "FATAL ERROR: Invalid state in %s: %d\n",
                        __func__, state);
                exit(1);
        }
    }

    // process the command options after this.
    show_cmdline_help(cfg);
}



/*********************************************
 * Callbacks used by the built-in command options.
 */

void show_cmdline_help(config_t* cfg) {

    cmdline_entry_t* ptr;
    char tmp[128];

    show_cmdline_vers(cfg);
    printf("%s\n\n", cfg->cmdline->preamble);
    printf("Use: %s [options] %s\n\n", cfg->pname,
            (cfg->cmdline->files > 1)? "[list of files]":
            (cfg->cmdline->files == 1)? "[file name]": "");

    memset(tmp, 0, sizeof(tmp));
    memset(tmp, '-', 80);
    printf("%s\n", tmp);

    for(ptr = cfg->cmdline->first; ptr != NULL; ptr = ptr->next) {
        if(ptr->short_opt != 0 || ptr->long_opt != NULL) {
            // this is an actual option.
            if(ptr->short_opt != 0)
                printf("  -%c ", ptr->short_opt);
            else
                printf("   ");

            if(ptr->long_opt != NULL)
                printf("--%-10s [", ptr->long_opt);
            else {
                printf("            ");
            }

            memset(tmp, 0, sizeof(tmp));
            if(ptr->type & CMD_LIST)
                strcpy(tmp, "list of ");
            if(ptr->type & CMD_BOOL)
                strcat(tmp, "bool]   ");
            else if(ptr->type & CMD_STR)
                strcat(tmp, "strg]   ");
            else if(ptr->type & CMD_NUM)
                strcat(tmp, "num]    ");
            else
                strcat(tmp, "]   ");
            printf(tmp);

            if(ptr->help != NULL)
                printf("%s %s\n", ptr->help, ptr->type & CMD_REQD? "(reqd)":"");
        }
        else if(ptr->type & CMD_DIV) {
            // this is a divider
            memset(tmp, 0, sizeof(tmp));
            memset(tmp, '-', 80);
            printf("%s\n", tmp);
        }
        else {
            // it's a file list
            if(ptr->help != NULL)
                printf("  %s %s\n", ptr->help, ptr->type & CMD_REQD? "(reqd)":"");
        }
    }

    memset(tmp, 0, sizeof(tmp));
    memset(tmp, '-', 80);
    printf("%s\n\n", tmp);
}

void show_cmdline_vers(config_t* cfg) {

    printf("%s: v%s\n", cfg->cmdline->name, cfg->cmdline->vers);
}

/*********************************************
 * APIs called by the user's application
 */

void init_cmdline(config_t* cfg, const char* name, const char* preamble,
                    const char* vers) {

    cfg->cmdline = _ALLOC_DS(cmdline_t);
    cfg->cmdline->preamble = _DUP_STR(preamble);
    cfg->cmdline->vers = _DUP_STR(vers);
    cfg->cmdline->name = _DUP_STR(name);
    cfg->cmdline->files = 0;
    cfg->cmdline->first = NULL;
    cfg->cmdline->last = NULL;
}

void add_cmdline(config_t* cfg, int short_opt, const char* long_opt,
                const char* name, const char* help, const char* def_val,
                cmdline_callback_t cb, cmdline_type_t type) {

    cmdline_entry_t* ptr = _ALLOC_DS(cmdline_entry_t);

    if(!(type & CMD_DIV) && short_opt == 0 && long_opt == NULL) {
        if(type & CMD_LIST)
            cfg->cmdline->files = 2;
        else
            cfg->cmdline->files = 1;
    }

    if(long_opt != NULL)
        ptr->long_opt = _DUP_STR(long_opt);
    else
        ptr->long_opt = NULL;

    if(def_val != NULL)
        ptr->def_val = _DUP_STR(def_val);
    else
        ptr->def_val = NULL;

    if(name != NULL)
        ptr->name = _DUP_STR(name);
    else
        ptr->name = NULL;

    if(help != NULL)
        ptr->help = _DUP_STR(help);
    else
        ptr->help = NULL;

    ptr->short_opt = short_opt;
    ptr->cb = cb;
    ptr->type = type;
    ptr->next = NULL;

    if(cfg->cmdline->last != NULL)
        cfg->cmdline->last->next = ptr;
    else
        cfg->cmdline->first = ptr;
    cfg->cmdline->last = ptr;
}
