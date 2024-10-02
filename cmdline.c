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
        fprintf(stderr, "CMDLINE: Expected %s but got '%c'\n\n", (s), get_char()); \
    } while(0)

#define ERROR(s, ...) do { \
        fprintf(stderr, "CMDLINE: "); \
        fprintf(stderr, s __VA_OPT__(,) __VA_ARGS__ ); \
        fprintf(stderr, "\n\n"); \
    } while(0)


/*********************************************
 * Private functions
 */
static const char** cmds = NULL;
static int cmds_idx = 0;
static int max_cmds_idx = 0;

static void show_cmdline_vers(config_t* cfg) {

    printf("%s: v%s\n", cfg->cmdline->name, cfg->cmdline->vers);
}

static void show_cmdline_help(config_t* cfg) {

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
                printf("     ");

            if(ptr->long_opt != NULL)
                printf("--%-10s [", ptr->long_opt);
            else {
                printf("             [");
            }

            memset(tmp, 0, sizeof(tmp));
            if(ptr->type & CMD_LIST)
                strcpy(tmp, "list of ");
            if(ptr->type & CMD_BOOL)
                strcat(tmp, "bool] ");
            else if(ptr->type & CMD_STR)
                strcat(tmp, "strg] ");
            else if(ptr->type & CMD_NUM)
                strcat(tmp, "num]  ");
            else
                strcat(tmp, "sw]   ");
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

static void init_cmd(int argc, char** argv) {

    cmds_idx = 0;
    max_cmds_idx = argc;
    cmds = (const char**)argv;
}

static const char* get_cmd(void) {

    if(cmds_idx >= max_cmds_idx)
        return NULL;
    else
        return cmds[cmds_idx];
}

static const char* consume_cmd(void) {

    if(cmds_idx < max_cmds_idx)
        cmds_idx++;

    return get_cmd();
}

/*
 * Read the list that was created with add_cmdline() and return the option if
 * it exists.
 */
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

/*
 * Read the list that was created with add_cmdline() and return the option if
 * it exists.
 */
static cmdline_entry_t* get_long_opt(config_t* cfg, const char* str) {

    cmdline_entry_t* ptr = cfg->cmdline->first;
    while(ptr != NULL) {
        if(ptr->long_opt != NULL && !strcmp(str, ptr->long_opt))
            break;
        else
            ptr = ptr->next;
    }

    return ptr;
}

/*
 * Search the command options for the given name and return the option if it
 * exists. Otherwise return NULL.
 */
static cmdline_entry_t* get_opt_by_name(config_t* cfg, const char* str) {

    cmdline_entry_t* ptr = cfg->cmdline->first;
    while(ptr != NULL) {
        if(ptr->name != NULL && !strcmp(str, ptr->name))
            break;
        else
            ptr = ptr->next;
    }

    return ptr;
}

/*
 * Search the command options for the given name and return the option if it
 * exists. Otherwise return NULL.
 */
static cmdline_entry_t* get_raw_list(config_t* cfg) {

    cmdline_entry_t* ptr = cfg->cmdline->first;
    while(ptr != NULL) {
        if(ptr->short_opt == 0 && ptr->long_opt == NULL && (!(ptr->type & CMD_DIV)))
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
 * Add a parsed command line value to the value database.
 */
static void add_cmdline_arg(config_t* cfg, cmdline_entry_t* item, const char* str) {

    printf("adding: %s = %s\n", item->name, str);
    // handle dups
}

/*
 * Parse a short option. Examples:
 * A short option can require an argument or not. If a arg is specified, then
 * whatever follows it is considered to be that argument.
 *
 * No args required.
 * -abcd
 * is equivalent to -a -b -c -d
 *
 * "-a" requires args
 * -abc something
 * is equivalent to -a bc something
 *
 * "-a" does not require args, but "-c" does, then
 * -abc something
 * is equivalent to -a -b -c something
 *
 * If "-a" requires and arg then
 * -a=10
 * is equivalent to -a 10
 * Else if "-a" does not require an arg, then it's a syntax error.
 *
 * If "-a" requires an arg then
 * -a10
 * is equivalent to -a=10
 * Else "-1" is an invalid option.
 *
 * If "-a" requires an arg then
 * -a-10
 * is equivalent to -a=-10
 * Else "-1" is an invalid option.
 *
 */
static void parse_short_option(config_t* cfg, const char* str) {

    //printf("sstr: %s\n", str);

    cmdline_entry_t* item;
    int idx = 0;
    int finished = 0;

    while(!finished) {

        item = get_short_opt(cfg, str[idx]);
        if(item != NULL) {
            if(item->cb != NULL) {
                (*item->cb)(cfg);
                return;
            }

            //printf("setting 'seen' on %s\n", item->name);
            item->type |= CMD_SEEN;
            if(item->type & CMD_ARGS) {
                if(str[idx+1] == '=' && str[idx+2] != '\0') {
                    printf("here\n");
                    add_cmdline_arg(cfg, item, &str[idx+2]);
                    consume_cmd();
                    return;
                }
                else if(str[idx+1] != '\0') {
                    add_cmdline_arg(cfg, item, &str[idx+1]);
                    consume_cmd();
                    return;
                }
                else {
                    str = consume_cmd();
                    if(str != NULL) {
                        add_cmdline_arg(cfg, item, str);
                        consume_cmd();
                        return;
                    }
                    else {
                        ERROR("short command option \"-%c\" requires argument", str[idx]);
                        show_cmdline_help(cfg);
                        exit(1);
                    }
                }
            }
        }
        else {
            ERROR("unknown short command option: \"-%c\"", str[idx]);
            show_cmdline_help(cfg);
            exit(1);
        }

        idx++;
        if(str[idx] == '\0')
            finished++;
    }

    consume_cmd();
}

/*
 * Parse a long option.
 *
 * A long option can require an argument or not and it follows the same
 * general rules as a short option. but the difference is that a long option
 * cannot be combined as a short option.
 *
 * if "--abc" does not require an arg then
 * -abc something
 * is a separate set of options, and
 * --abc=something
 * is a syntax error.
 *
 * If "--abc" does require an arg, then
 * --abc=something
 * and
 * --abc something
 * are equivalent.
 *
 */
static void parse_long_option(config_t* cfg, const char* str) {

    //printf("lstr: %s\n", str);

    char* tpt = _DUP_STR(str);
    char* arg = NULL;

    if(NULL != (arg = strchr(tpt, '='))) {
        *arg = '\0';
        arg++;
    }

    cmdline_entry_t* item = get_long_opt(cfg, tpt);

    if(item != NULL) {
        if(item->cb != NULL) {
            _FREE(tpt);
            (*item->cb)(cfg);
            return;
        }

        //printf("setting 'seen' on %s\n", item->name);
        item->type |= CMD_SEEN;
        if(item->type & CMD_ARGS) {
            if(arg == NULL) {
                const char* str = consume_cmd();
                if(str != NULL) {
                    add_cmdline_arg(cfg, item, str);
                    consume_cmd();
                    _FREE(tpt);
                    return;
                }
                else {
                    ERROR("long command option \"--%s\" requires argument", tpt);
                    show_cmdline_help(cfg);
                    exit(1);
                }
            }
            else if(arg[0] != '\0') {
                add_cmdline_arg(cfg, item, arg);
                consume_cmd();
                _FREE(tpt);
                return;
            }
            else {
                ERROR("expected an argument after \"--%s\"", tpt);
                show_cmdline_help(cfg);
                exit(1);
            }
        }
    }
    else {
        ERROR("unknown long command option: \"--%s\"", tpt);
        show_cmdline_help(cfg);
        exit(1);
    }

    _FREE(tpt);
    consume_cmd();
}


/*
 * When this is entered, a command line item has been encountered that does not
 * start with a '-'. If there is a option that allows a raw list then the
 * string is stored as it is found. otherwise, it is an "unknown option"
 * error.
 *
 * Note that an exception is made when the raw option starts with a '-' but
 * there are spaces in it. That means that it had quotes around it. and so it
 * is a single entity.
 */
static void parse_list_item(config_t* cfg, const char* str) {

    cmdline_entry_t* item = get_raw_list(cfg);

    //printf("rstr: %s\n", str);
    if(item != NULL) {
        add_cmdline_arg(cfg, item, str);
        item->type |= CMD_SEEN;
    }
    else {
        ERROR("unknown option: \"%s\"", str);
        show_cmdline_help(cfg);
        exit(1);
    }
    consume_cmd();
}

/*
 * Parse the command line.
 *
 * Possible bug:
 * Bash strips quotes around items. If the item is in quotes and starts with a
 * dash (-) then it is mistaken for a command option. This could be a problem
 * if it's needed to specify command parameters for a child program. One
 * solution may be to have the parser expect to ignore dashes in the value. But
 * that leads to other possible issues.
 *
 * Question:
 * Does winders also strip quotes? Are there issues with not using a slash (/)
 * to intro command options?
 */
void parse_cmdline(config_t* cfg, int argc, char** argv) {

    init_cmd(argc, argv);

    const char* ptr = consume_cmd(); // discard the first element.

    while(1) {
        if(NULL != (ptr = get_cmd())) {
            if(ptr[0] == '-') {
                // parse the item with the leading '-' removed
                if(ptr[1] == '-')
                    parse_long_option(cfg, &ptr[2]);
                else
                    parse_short_option(cfg, &ptr[1]);
            }
            else {
                parse_list_item(cfg, ptr);
            }
        }
        else
            break;
    }

    // process the required command options after this.
    cmdline_entry_t* opt = cfg->cmdline->first;
    while(opt != NULL) {
        if(opt->type & CMD_REQD && !(opt->type & CMD_SEEN)) {
            if(opt->long_opt != NULL)
                ERROR("required command option not found: \"--%s\"", opt->long_opt);
            else if(opt->short_opt != 0)
                ERROR("required command option not found: \"-%c\"", opt->short_opt);
            else if(opt->name != NULL)
                ERROR("required command option not found: \"%s\"", opt->name);

            show_cmdline_help(cfg);
            exit(1);
        }
        opt = opt->next;
    }
}



/*********************************************
 * Callbacks used by the built-in command options.
 */

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

    if(type & CMD_LIST)
        ptr->type |= CMD_ARGS;

    ptr->next = NULL;

    if(cfg->cmdline->last != NULL)
        cfg->cmdline->last->next = ptr;
    else
        cfg->cmdline->first = ptr;
    cfg->cmdline->last = ptr;
}

void cb_cmdline_help(config_t* cfg) {

    show_cmdline_help(cfg);
    exit(1);
}

void cb_cmdline_vers(config_t* cfg) {

    show_cmdline_vers(cfg);
    exit(1);
}
