#ifndef __READLINE_CONTEXT_H__
#define __READLINE_CONTEXT_H__

#include "readline.h"
#include "history.h"

#include <termios.h>
#include <stdbool.h>

struct readline_st
{
    int out_fd;
    int in_fd;
    bool is_a_terminal;
    unsigned int maximum_seconds_to_wait_for_char;
    int terminal_width;
    struct termios previous_terminal_settings;
    char const * prompt;

    line_context_st line_context;
    char const * saved_line;
    history_st * history;
    void * user_completion_context;    /* user specific context passed back to the user when performing auto-complete */
    bool insert_mode;
    completion_callback_fn completion_callback;
}; 

#endif /* __READLINE_CONTEXT_H__ */
