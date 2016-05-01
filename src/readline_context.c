/* Copyright (C) Chris Nisbet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly 
 * prohibited. Proprietary and confidential. Written by Chris 
 * Nisbet <nisbet@ihug.co.nz>, April 2016.
 */

#include "readline_context.h"

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
    free((void *)readline_ctx->field_separators);
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

bool readline_context_history_control(readline_st * const readline_ctx, bool const enable)
{
    bool const previous_enable_state = readline_ctx->history_enabled;

    readline_ctx->history_enabled = enable;

    return previous_enable_state;
}

char readline_context_mask_character_control(readline_st * const readline_ctx, char const mask_character)
{
    char const previous_mask_character = readline_ctx->mask_character;

    readline_ctx->mask_character = mask_character;

    return previous_mask_character;
}

void readline_context_set_field_separators(readline_st * const readline_ctx, char const * const field_separators)
{
    free((void *)readline_ctx->field_separators);
    readline_ctx->field_separators = strdup(field_separators);
}

size_t readline_context_set_maximum_line_length(readline_st * const readline_ctx, size_t const maximum_line_length)
{
    size_t const previous_maximum = readline_ctx->maximum_line_length;

    readline_ctx->maximum_line_length = maximum_line_length;

    return previous_maximum;
}
