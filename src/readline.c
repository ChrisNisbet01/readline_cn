#include "readline.h"
#include "args.h"
#include "terminal.h"
#include "line_context.h"
#include "readline_context.h"
#include "tokenise.h"
#include "history.h"
#include "readline_status.h"
#include "read_char.h"
#include "handlers.h"
#include "terminal_cursor.h"

#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#define INITIAL_LINE_BUFFER_SIZE 10
#define LINE_BUFFER_SIZE_INCREMENT 5

#define BACKSPACE 127
#define ESC 27


static readline_status_t handle_new_input_from_terminal(readline_st * const readline_ctx, int const ch)
{
    readline_status_t status;

    switch (ch)
    {
        case ESC:
            status = handle_escaped_char(readline_ctx);
            break;
        case BACKSPACE:
            handle_backspace(readline_ctx);
            status = readline_status_continue;
            break;
        default:
            if (ISCTL(ch))
            {
                status = handle_control_char(readline_ctx, ch);
            }
            else
            {
                handle_regular_char(readline_ctx, ch, true);
                status = readline_status_continue;
            }
            break;
    }

    return status;
}

static readline_status_t handle_new_input_from_file(readline_st * const readline_ctx, int const ch)
{
    readline_status_t status;

    switch (ch)
    {
        case '\n':
            status = readline_status_done;
            break;
        default:
            handle_regular_char(readline_ctx, ch, false);
            status = readline_status_continue;
            break;
    }

    return status;
}

static readline_status_t process_new_input(readline_st * const readline_ctx, int const ch)
{
    readline_status_t status;

    if (readline_ctx->is_a_terminal)
    {
        status = handle_new_input_from_terminal(readline_ctx, ch);
    }
    else
    {
        status = handle_new_input_from_file(readline_ctx, ch);
    }

    return status;
}

static readline_status_t get_and_process_new_input(readline_st * const readline_ctx)
{
    readline_status_t status;
    int ch;

    ch = read_char_from_input_descriptor(readline_ctx->in_fd,
                                         readline_ctx->maximum_seconds_to_wait_for_char,
                                         &status);
    if (status != readline_status_continue)
    {
        goto done;
    }
    status = process_new_input(readline_ctx, ch);

done:
    return status;
}

static readline_status_t edit_input(readline_st * const readline_ctx)
{
    readline_status_t status = readline_status_continue;

    if (readline_ctx->is_a_terminal)
    {
        line_context_st * const line_ctx = &readline_ctx->line_context;

        terminal_puts(&line_ctx->terminal_cursor, line_ctx->prompt, '\0');
    }

    do
    {
        status = get_and_process_new_input(readline_ctx);
    }
    while (status == readline_status_continue);

    return status;
}

static bool readline_init(readline_st * const readline_ctx, 
                          char const * const prompt, 
                          size_t const timeout_seconds)
{
    bool readline_prepared;
    line_context_st * const line_ctx = &readline_ctx->line_context;
    size_t terminal_width;

    readline_ctx->maximum_seconds_to_wait_for_char = timeout_seconds;

    readline_ctx->terminal_was_modified = false;
    if (readline_ctx->is_a_terminal)
    {
        terminal_width = terminal_get_width(readline_ctx->out_fd);

        history_reset(readline_ctx->history);

        terminal_restore(&readline_ctx->previous_terminal_settings);
        readline_ctx->terminal_was_modified = true;
    }
    else
    {
        terminal_width = 0;
    }

    free_saved_line(&readline_ctx->saved_line); 

    if (!line_context_init(line_ctx,
                           INITIAL_LINE_BUFFER_SIZE,
                           LINE_BUFFER_SIZE_INCREMENT,
                           readline_ctx->maximum_line_length,
                           readline_ctx->out_fd,
                           terminal_width,
                           readline_ctx->mask_character,
                           prompt))
    {
        readline_prepared = false;
        goto done;
    }

    readline_prepared = true;

done:
    return readline_prepared;
}

static void readline_cleanup(readline_st * const readline_ctx)
{
    line_context_st * const line_ctx = &readline_ctx->line_context;

    if (readline_ctx->terminal_was_modified)
    {
        terminal_restore(&readline_ctx->previous_terminal_settings);
    }

    line_context_teardown(line_ctx);
    free_saved_line(&readline_ctx->saved_line);
}

readline_result_t readline(readline_st * const readline_ctx, unsigned int const timeout_seconds, char const * const prompt, char * * const line)
{
    readline_status_t readline_status;
    readline_result_t readline_result;
    line_context_st * const line_ctx = &readline_ctx->line_context;
    bool should_return_line = false;

    if (!readline_init(readline_ctx, prompt, timeout_seconds))
    {
        readline_result = readline_result_error;
        goto done;
    }

    readline_status = edit_input(readline_ctx);

    switch(readline_status)
    {
        case readline_status_done:
        {
            should_return_line = true;
            readline_result = readline_result_success;
            break;
        }
        case readline_status_continue:  /* Shouldn't happen, but if it does, call it an error. */
            /* drop through */
        case readline_status_error:
            readline_result = readline_result_error;
            break;
        case readline_status_eof:
            should_return_line = true;
            readline_result = readline_result_eof;
            break;
        case readline_status_ctrl_c:
            readline_result = readline_result_ctrl_c;
            break;
        case readline_status_timed_out:
            readline_result = readline_result_timed_out;
            break;
        default:
            /* Here just to prevent a compiler warning. */
            readline_result = readline_result_error;
            break;
    }

done:
    if (should_return_line)
    {
        bool const should_add_to_history = readline_ctx->history_enabled &&
            readline_ctx->is_a_terminal &&
            readline_ctx->mask_character == '\0';

        if (should_add_to_history)
        {
            history_add(readline_ctx->history, line_ctx->buffer);
        }
        *line = line_ctx->buffer;
        line_ctx->buffer = NULL;
    }
    else
    {
        *line = NULL;
    }

    readline_cleanup(readline_ctx);

    return readline_result;
}

static tokens_st * parse_tokens_from_line(char const * const line, char const * const field_separators)
{
    tokens_st * tokens;

    tokens = tokenise_line(line, 0, 0, false, field_separators);

    return tokens;
}

static args_st * get_args_from_tokens(tokens_st const * const tokens)
{
    size_t index;
    args_st * args;
    size_t const token_count = tokens_get_num_tokens(tokens);

    args = args_alloc(tokens_get_num_tokens(tokens) + 1);   /* include 1 for the NULL terminator */
    if (args == NULL)
    {
        goto done;
    }
    for (index = 0; index < token_count; index++)
    {
        args_add_arg(args, tokens_get_token_at_index(tokens, index));
    }
    args_add_arg(args, NULL);

done:
    return args;
}

/* this version of readline returns a set of args rather than 
 * just the line. 
 */
readline_result_t readline_args(readline_st * const readline_ctx, 
                                unsigned int const timeout_seconds,
                                char const * const prompt, 
                                size_t * const argc, 
                                char const * * * argv)
{
    readline_result_t result;
    char * line;
    tokens_st * tokens = NULL;
    args_st * args;

    result = readline(readline_ctx, timeout_seconds, prompt, &line);
    if (line == NULL)
    {
        args = NULL;
        goto done;
    }

    tokens = parse_tokens_from_line(line, readline_ctx->field_separators);
    if (tokens == NULL)
    {
        args = NULL;
        goto done;
    }

    args = get_args_from_tokens(tokens);

done:

    free(line);
    tokens_free(tokens);
    if (args != NULL)
    {
        *argc = args->argc;
        *argv = args->argv;
        args->argv = NULL;
    }
    else
    {
        *argv = NULL;
    }
    args_free(args);

    return result;
}

