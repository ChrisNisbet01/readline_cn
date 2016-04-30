#ifndef __READLINE_CONTEXT_H__
#define __READLINE_CONTEXT_H__

#include "readline.h"
#include "history.h"

#include <termios.h>
#include <stdbool.h>

struct readline_st
{
    int out_fd; /* File descriptor to write to. */
    int in_fd; /* File descriptor to read from. */
    unsigned int maximum_seconds_to_wait_for_char;
    size_t maximum_line_length;

    bool is_a_terminal;
    bool terminal_was_modified;
    struct termios previous_terminal_settings;

    bool insert_mode; 
    int mask_character; /* if non-zero, the terminal writes out this character rather than the actual character. */
    char const * field_separators;

    line_context_st line_context;

    char const * saved_line;
    bool history_enabled;
    history_st * history;

    void * user_context; /* user specific context passed back to the user when performing auto-complete or help */
    completion_callback_fn completion_callback;
    help_callback_fn help_callback;
    char help_key; /* Usually a '?'. Calls the help callback if not NULL. */
};

#endif /* __READLINE_CONTEXT_H__ */
