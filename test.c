/*
 * Test the configuration stuff.
 */
#include <stdio.h>

#include "config.h"
#include "cmdline.h"

int main(int argc, char** argv, char** envp) {

    config_t* cfg = init_configuration("Config Test", 
        "This is the test program for the configuration loader", "0.0.0.0.1");

    
    add_cmdline(cfg, 'i', "ipaddr", "ipaddr", "the IP address to use", "localhost", NULL, CMD_STR|CMD_RARG);
    add_cmdline(cfg, 'x', "xtra", "xtra", "this is the extra parameter", "blart", NULL, CMD_NUM|CMD_LIST|CMD_REQD);
    
    add_cmdline(cfg, 0, NULL, NULL, NULL, NULL, NULL, CMD_DIV);
    add_cmdline(cfg, 'v', "verbosity", NULL, "show progress as program executes", NULL, NULL, CMD_NUM|CMD_RARG);
    add_cmdline(cfg, 'V', "version", NULL, "print the program version", NULL, show_cmdline_vers, CMD_BOOL);
    add_cmdline(cfg, 'h', "help", NULL, "print this help text", NULL, show_cmdline_help, CMD_BOOL);

    add_cmdline(cfg, 0, NULL, NULL, NULL, NULL, NULL, CMD_DIV);
    add_cmdline(cfg, 0, NULL, "files", "list of files to be processed", NULL, NULL, CMD_REQD|CMD_LIST);    

    load_configuration(cfg, argc, argv, envp);
    //dump_hash_table(cfg->vars);
    return 0;
}
