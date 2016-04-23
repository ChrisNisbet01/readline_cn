#include "readline.h"
#include "args.h"
#include "terminal.h"
#include "line_context.h"
#include "readline_context.h"
#include "tokenise.h"
#include "word_completion.h"
#include "help.h"
#include "history.h"

#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#define INITIAL_LINE_BUFFER_SIZE 10
#define LINE_BUFFER_SIZE_INCREMENT 5

typedef enum readline_status_t readline_status_t;
enum readline_status_t
{
    readline_status_done,
    readline_status_error,
    readline_status_continue,
    readline_status_ctrl_c,
    readline_status_timed_out,
    readline_status_eof
}; 

#define BACKSPACE 127
#define ESC 27

static char       newline_str[] = "\n";

static void handle_right_arrow(readline_st * const readline_ctx)
{
    line_context_st * const line_ctx = &readline_ctx->line_context;

    move_cursor_right_n_columns(line_ctx, 1);
}

static void handle_left_arrow(readline_st * const readline_ctx)
{
    line_context_st * const line_ctx = &readline_ctx->line_context;

    move_cursor_left_n_columns(line_ctx, 1);
}

static void handle_home_key(readline_st * const readline_ctx)
{
    line_context_st * const line_ctx = &readline_ctx->line_context;

    move_cursor_left_n_columns(line_ctx, line_ctx->cursor_index);
}

static void handle_end_key(readline_st * const readline_ctx)
{
    line_context_st * const line_ctx = &readline_ctx->line_context;

    move_cursor_right_n_columns(line_ctx, line_ctx->line_length - line_ctx->cursor_index);
}

static void handle_insert_key(readline_st * const readline_ctx)
{
    readline_ctx->insert_mode = !readline_ctx->insert_mode;
    /* FIXME - Change the cursor shape according to current mode. */
}

static void handle_backspace(readline_st * const readline_ctx)
{
    line_context_st * const line_ctx = &readline_ctx->line_context;

    delete_char_to_the_left(line_ctx);
}

static void handle_delete(readline_st * const readline_ctx)
{
    line_context_st * const line_ctx = &readline_ctx->line_context;

    delete_char_to_the_right(line_ctx, true);
}

static void save_current_line(line_context_st * const line_ctx, char const * * const destination)
{
    /* Free any old string at the destination. */
    free((void *)*destination);
    *destination = strdup(line_ctx->buffer);
}

static void free_saved_line(char const * * const saved_line)
{
    free((void *)*saved_line);
    *saved_line = NULL;
}

static void handle_up_arrow(readline_st * const readline_ctx)
{
    history_st * const history = readline_ctx->history;
    line_context_st * const line_ctx = &readline_ctx->line_context;
    char const * historic_line;

    if (history_currently_at_most_recent(readline_ctx->history))
    {
        /* save the current line in case the user returns to the top 
         * of the history 
         */
        save_current_line(line_ctx, &readline_ctx->saved_line);

    }
    historic_line = history_get_older_entry(history);

    if (historic_line != NULL)
    {
        replace_line(line_ctx, historic_line);
    }

}

static void handle_down_arrow(readline_st * const readline_ctx)
{
    history_st * const history = readline_ctx->history;
    char const * replacement_line;

    replacement_line = history_get_newer_entry(history);
    if (replacement_line == NULL)
    {
        if (history_currently_at_most_recent(history))
        {
            if (readline_ctx->saved_line != NULL)
            {
                line_context_st * line_ctx = &readline_ctx->line_context;

                /* If the current line matched what we had saved and we still 
                 * replaced it, we'd get the side-effect that the current 
                 * cursor editing poistion would jump to the end of the line. 
                 */
                if (strcmp(line_ctx->buffer, readline_ctx->saved_line) != 0)
                {
                    replacement_line = readline_ctx->saved_line;
                }
            }
        }
    }

    if (replacement_line != NULL)
    {
        line_context_st * line_ctx = &readline_ctx->line_context;

        replace_line(line_ctx, replacement_line);
        if (replacement_line == readline_ctx->saved_line)
        {
            free_saved_line(&readline_ctx->saved_line);
        }
    }
}

static readline_status_t handle_escaped_char(readline_st * const readline_ctx)
{
    readline_status_t status = readline_status_continue;
    int escaped_char;
    tty_get_result_t tty_get_result;

    tty_get_result = tty_get(readline_ctx->in_fd, readline_ctx->maximum_seconds_to_wait_for_char, &escaped_char);
    switch (tty_get_result)
    {
        case tty_get_result_eof:
            status = readline_status_eof;
            goto done; 

        case tty_get_result_timeout:
            status = readline_status_timed_out;
            goto done;
        case tty_get_result_ok:
            break;
    }

    if (escaped_char == '[' || escaped_char == 'O')
    {
        int escape_command_char;

        tty_get_result = tty_get(readline_ctx->in_fd, readline_ctx->maximum_seconds_to_wait_for_char, &escape_command_char);
        switch (tty_get_result)
        {
            case tty_get_result_eof:
                status = readline_status_eof;
                goto done;

            case tty_get_result_timeout:
                status = readline_status_timed_out;
                goto done;
            case tty_get_result_ok:
                break;
        }

        switch (escape_command_char)
        {
            case '1':
                tty_get(readline_ctx->in_fd, readline_ctx->maximum_seconds_to_wait_for_char, NULL);  /* this sequence includes a trailing '~' char */
                handle_home_key(readline_ctx);
                break;
            case '2':
                tty_get(readline_ctx->in_fd, readline_ctx->maximum_seconds_to_wait_for_char, NULL);  /* this sequence includes a trailing '~' char */
                handle_insert_key(readline_ctx);
                break;
            case '3':
                tty_get(readline_ctx->in_fd, readline_ctx->maximum_seconds_to_wait_for_char, NULL);  /* this sequence includes a trailing '~' char */
                handle_delete(readline_ctx);
                break;
            case '4':
                tty_get(readline_ctx->in_fd, readline_ctx->maximum_seconds_to_wait_for_char, NULL);  /* this sequence includes a trailing '~' char */
                handle_end_key(readline_ctx);
                break;
            case '5':
                tty_get(readline_ctx->in_fd, readline_ctx->maximum_seconds_to_wait_for_char, NULL);  /* this sequence includes a trailing '~' char */
                // TODO: handle_page_up(readline_ctx);
                break;
            case '6':
                tty_get(readline_ctx->in_fd, readline_ctx->maximum_seconds_to_wait_for_char, NULL);  /* this sequence includes a trailing '~' char */
                // TODO: handle_page_down(readline_ctx);
                break;
            case 'A':
                handle_up_arrow(readline_ctx);
                break;
            case 'B':
                handle_down_arrow(readline_ctx);
                break;
            case 'C':
                handle_right_arrow(readline_ctx);
                break;
            case 'D':
                handle_left_arrow(readline_ctx);
                break;
            case 'E':   /* 5 on the numeric keypad */
                break;
            case 'F':
                handle_end_key(readline_ctx);
                break;
            case 'H':
                handle_home_key(readline_ctx);
                break;
            default:
                break;
        }
    }

done:
    return status;
}

static readline_status_t handle_enter(readline_st const * const readline_ctx)
{
    tty_puts(readline_ctx->out_fd, newline_str, '\0');

    return readline_status_done;
}

static void handle_tab(readline_st * const readline_ctx)
{
    /* Word completion isn't performed if the user has requested 
     * to mask the input characters. 
     */
    if (readline_ctx->mask_character == '\0')
    {
        do_word_completion(readline_ctx);
    }
}

static readline_status_t handle_control_char(readline_st * const readline_ctx, int const ch)
{
    readline_status_t status = readline_status_continue;

    switch (ch)
    {
        case '\t': /* TAB */
            handle_tab(readline_ctx); 
            break;
        case '\n': /* ENTER */
            status = handle_enter(readline_ctx);
            break;
        case CTL('C'):
            status = readline_status_ctrl_c;
            break;
        default:  /* silently ignore */
            break;
    }                 

    return status;
}

static void handle_regular_char(readline_st * const readline_ctx, int const ch, bool const update_terminal)
{
    if (readline_ctx->help_key != '\0' && ch == (int)readline_ctx->help_key)
    {
        do_help(readline_ctx);
    }
    else
    {
        write_char(&readline_ctx->line_context, ch, readline_ctx->insert_mode, update_terminal);
    }
}

static readline_status_t get_new_input_from_terminal(readline_st * const readline_ctx)
{
    int c;
    readline_status_t status;
    tty_get_result_t tty_get_result;

    tty_get_result = tty_get(readline_ctx->in_fd, readline_ctx->maximum_seconds_to_wait_for_char, &c);
    switch (tty_get_result)
    {
        case tty_get_result_eof:
            status = readline_status_eof;
            goto done;

        case tty_get_result_timeout:
            status = readline_status_timed_out;
            goto done;
        case tty_get_result_ok:
            break;
    }

    switch (c)
    {
        case ESC:
            status = handle_escaped_char(readline_ctx);
            break;
        case BACKSPACE:
            handle_backspace(readline_ctx);
            status = readline_status_continue;
            break;
        default:
            if (ISCTL(c))
            {
                status = handle_control_char(readline_ctx, c);
            }
            else
            {
                handle_regular_char(readline_ctx, c, true);
                status = readline_status_continue;
            }
            break;
    }

done:
    return status;
}

static readline_status_t get_new_input_from_file(readline_st * const readline_ctx)
{
    int c;
    readline_status_t status;
    tty_get_result_t tty_get_result;

    tty_get_result = tty_get(readline_ctx->in_fd, readline_ctx->maximum_seconds_to_wait_for_char, &c);
    switch (tty_get_result)
    {
        case tty_get_result_eof:
            status = readline_status_eof;
            goto done;
        case tty_get_result_timeout:
            status = readline_status_timed_out;
            goto done;
        case tty_get_result_ok:
            break;
    }

    switch (c)
    {
        case '\n':
            status = readline_status_done;
            break;
        default:
            handle_regular_char(readline_ctx, c, false);
            status = readline_status_continue;
            break;
    }

done:
    return status;
}

static readline_status_t get_new_input(readline_st * const readline_ctx)
{
    readline_status_t status;

    if (readline_ctx->is_a_terminal)
    {
        status = get_new_input_from_terminal(readline_ctx);
    }
    else
    {
        status = get_new_input_from_file(readline_ctx);
    }

    return status;
}

static readline_status_t edit_input(readline_st * const readline_ctx)
{
    readline_status_t status = readline_status_continue;

    if (readline_ctx->is_a_terminal)
    {
        tty_puts(readline_ctx->out_fd, readline_ctx->prompt, '\0');
    }

    do
    {
        status = get_new_input(readline_ctx);
    }
    while (status == readline_status_continue);

    return status;
}

static bool readline_init(readline_st * const readline_ctx)
{
    bool readline_prepared;
    line_context_st * const line_ctx = &readline_ctx->line_context;

    if (!line_context_init(line_ctx,
                          INITIAL_LINE_BUFFER_SIZE,
                          LINE_BUFFER_SIZE_INCREMENT,
                          readline_ctx->out_fd,
                          readline_ctx->mask_character))
    {
        readline_prepared = false;
        goto done;
    }

    if (readline_ctx->is_a_terminal)
    {
        readline_ctx->terminal_width = get_terminal_width(readline_ctx->out_fd);

        history_reset(readline_ctx->history);

        prepare_terminal(&readline_ctx->previous_terminal_settings);
    }

    free_saved_line(&readline_ctx->saved_line);

    readline_prepared = true;

done:
    return readline_prepared;
}

static void readline_cleanup(readline_st * const readline_ctx)
{
    line_context_st * const line_ctx = &readline_ctx->line_context;

    if (readline_ctx->is_a_terminal)
    {
        restore_terminal(&readline_ctx->previous_terminal_settings);
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

    readline_ctx->maximum_seconds_to_wait_for_char = timeout_seconds;
    readline_ctx->prompt = prompt; 

    if (!readline_init(readline_ctx))
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
    }

    if (should_return_line)
    {
        bool const should_add_to_history = readline_ctx->history_enabled &&
            readline_ctx->is_a_terminal &&
            readline_ctx->mask_character == '\0';

        if (should_add_to_history)
        {
            history_st * history = readline_ctx->history;

            history_add(history, line_ctx->buffer);
        }
        *line = line_ctx->buffer;
        line_ctx->buffer = NULL;
    }
    else
    {
        *line = NULL;
    }

    readline_cleanup(readline_ctx);

done:

    return readline_result;
}

static tokens_st * parse_tokens_from_line(char const * const line)
{
    tokens_st * tokens;

    tokens = tokenise_line(line, 0, 0, false);

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

    tokens = parse_tokens_from_line(line);
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

