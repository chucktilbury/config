/*
 * Command line public interface.
 */
#ifndef _CMDLINE_H_
#define _CMDLINE_H_

#include "config.h"

typedef enum {
    // place holder
    CMD_NONE = 0,

    // needed by getarg()
    CMD_ARGS = 0x01, // args required

    // data type helpers
    CMD_STR = 0x02,  // type is a string
    CMD_NUM = 0x04,  // type is a number
    CMD_BOOL = 0x08, // type is bool, requires "true" or "false"
    CMD_LIST = 0x10, // a list is accepted by the arg. implies CMD_ARGS.

    // arg attributes
    CMD_REQD = 0x20, // item is required
    CMD_DIV = 0x40,  // item is a divider for the help screen

    // internal flags, do not use
    CMD_SEEN = 0x80,
} cmdline_type_t;

typedef void (*cmdline_callback_t)(config_t*);

// This is for options only. The values are stored in the config data
// structure. It's only used when parsing the command line.
typedef struct _cmdline_entry_t_ {
    int short_opt;
    const char* long_opt;
    const char* name;
    const char* help;
    const char* def_val;
    cmdline_callback_t cb;
    cmdline_type_t type;
    struct _cmdline_entry_t_* next;
} cmdline_entry_t;

typedef struct _cmdline_t_ {
    const char* preamble;
    const char* vers;
    const char* name;
    int files;
    struct _cmdline_entry_t_* first;
    struct _cmdline_entry_t_* last;
} cmdline_t;

void init_cmdline(config_t* cfg,
                const char* name,
                const char* preamble,
                const char* vers);
void add_cmdline(config_t* cfg,
                int short_opt,
                const char* long_opt,
                const char* name,
                const char* help,
                const char* def_val,
                cmdline_callback_t cb,
                cmdline_type_t type);

void destroy_cmdline(cmdline_t* cmd);
void parse_cmdline(config_t* cfg, int argc, char** argv);

void cb_cmdline_help(config_t* cfg);
void cb_cmdline_vers(config_t* cfg);

#endif /* _CMDLINE_H_ */
