/* Copyright (C) Chris Nisbet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly 
 * prohibited. Proprietary and confidential. Written by Chris 
 * Nisbet <nisbet@ihug.co.nz>, April 2016.
 */

#include "readline_context.h"
#include "utils.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static readline_st * readline_context_alloc(void)
{
    readline_st * readline_ctx;

    readline_ctx = (readline_st *)calloc(1, sizeof *readline_ctx);

    return readline_ctx;
}

static void readline_context_free(readline_st * const readline_ctx)
{
    FREE_CONST(readline_ctx->field_separators);
    line_context_teardown(&readline_ctx->line_context);
    history_free(readline_ctx->history);
    free_saved_string(&readline_ctx->saved_line);
    free(readline_ctx);
}

void toggle_insert_mode(readline_st * const readline_ctx)
{
    readline_ctx->insert_mode = !readline_ctx->insert_mode;
    /* TODO - Change the cursor shape according to current mode. */
}

readline_st * readline_context_create(void * const user_context, 
                                      completion_callback_fn const completion_callback,
                                      help_callback_fn const help_callback,
                                      char const help_key,
                                      int input_fd,
                                      int output_fd,
                                      size_t const history_size)
{
    readline_st * readline_ctx;

    readline_ctx = readline_context_alloc();
    if (readline_ctx == NULL)
    {
        goto done;
    }
    readline_ctx->insert_mode = true;

    readline_ctx->user_context = user_context;
    readline_ctx->completion_callback = completion_callback;
    readline_ctx->help_callback = help_callback;
    readline_ctx->help_key = help_key;
    readline_ctx->out_fd = output_fd;
    readline_ctx->in_fd = input_fd;
    readline_ctx->is_a_terminal = isatty(readline_ctx->in_fd);
    readline_ctx->history = history_alloc(history_size);
    readline_ctx->history_enabled = true;
    readline_ctx->check_timeout_before_any_chars_read = true;

done:
    return readline_ctx;
}

void readline_context_destroy(readline_st * const readline_ctx)
{
    if (readline_ctx != NULL)
    {
        readline_context_free(readline_ctx);
    }
}

bool readline_history_control(readline_st * const readline_ctx, bool const enable)
{
    bool previous_enable_state;

    if (readline_ctx != NULL)
    {
        previous_enable_state = readline_ctx->history_enabled;

        readline_ctx->history_enabled = enable;
    }
    else
    {
        /* XXX - return error? */
        previous_enable_state = false;
    }

    return previous_enable_state;
}

char readline_set_mask_character(readline_st * const readline_ctx, char const mask_character)
{
    char previous_mask_character;

    if (readline_ctx != NULL)
    {
        previous_mask_character = readline_ctx->mask_character;

        readline_ctx->mask_character = mask_character;
    }
    else
    {
        /* XXX - Return error? */
        previous_mask_character = '\0';
    }

    return previous_mask_character;
}

void readline_set_field_separators(readline_st * const readline_ctx, char const * const field_separators)
{
    if (readline_ctx != NULL)
    {
        FREE_CONST(readline_ctx->field_separators);
        if (field_separators != NULL)
        {
            readline_ctx->field_separators = strdup(field_separators);
        }
        else
        {
            readline_ctx->field_separators = NULL;
        }
    }
}

size_t readline_set_maximum_line_length(readline_st * const readline_ctx, size_t const maximum_line_length)
{
    size_t previous_maximum;

    if (readline_ctx != NULL)
    {
        previous_maximum = readline_ctx->maximum_line_length;

        readline_ctx->maximum_line_length = maximum_line_length;
    }
    else
    {
        /* XXX return error? */
        previous_maximum = 0;
    }

    return previous_maximum;
}

void readline_set_initial_timeout_check(readline_st * const readline_ctx, bool const do_initial_check)
{
    if (readline_ctx != NULL)
    {
        readline_ctx->check_timeout_before_any_chars_read = do_initial_check;
    }
}
