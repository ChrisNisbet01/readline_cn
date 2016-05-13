/* Copyright (C) Chris Nisbet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly 
 * prohibited. Proprietary and confidential. Written by Chris 
 * Nisbet <nisbet@ihug.co.nz>, April 2016.
 */

#ifndef __READLINE_CONTEXT_H__
#define __READLINE_CONTEXT_H__

#include "readline.h"
#include "history.h"
#include "terminal.h"

#include <stdbool.h>

/*  
 * This structure contains variables that need to persist 
 * between calls to readline(). 
*/ 
struct readline_st
{
    int out_fd; /* File descriptor to write to. */
    int in_fd; /* File descriptor to read from. */
    unsigned int maximum_seconds_to_wait_for_char;
    bool check_timeout_before_any_chars_read; /* set to false if there is no timeout before the user starts entering characters. */
    size_t maximum_line_length;

    bool is_a_terminal;
    bool terminal_was_modified;
    terminal_settings_st * previous_terminal_settings;

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
    char help_key; /* If set, would usually be to '?'. Calls the help callback if that's not NULL. */
};

#endif /* __READLINE_CONTEXT_H__ */
