#include "readline.h"
#include "args.h"
#include "terminal.h"
#include "line_context.h"
#include "readline_context.h"
#include "tokenise.h"
#include "word_completion.h"
#include "history.h"

#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#define INITIAL_LINE_BUFFER_SIZE 10
#define LINE_BUFFER_SIZE_INCREMENT 5

typedef enum editline_status_t editline_status_t;
enum editline_status_t
{
    editline_status_done,
    editline_status_error,
    editline_status_continue,
    editline_status_ctrl_c,
    editline_status_timed_out,
    editline_status_eof
}; 

#define BACKSPACE 127
#define ESC 27

static char       newline_str[] = "\n";

static void handle_right_arrow(editline_st * const editline_ctx)
{
    line_context_st * const line_ctx = &editline_ctx->line_context;

    move_cursor_right_n_columns(line_ctx, 1);
}

static void handle_left_arrow(editline_st * const editline_ctx)
{
    line_context_st * const line_ctx = &editline_ctx->line_context;

    move_cursor_left_n_columns(line_ctx, 1);
}

static void handle_home_key(editline_st * const editline_ctx)
{
    line_context_st * const line_ctx = &editline_ctx->line_context;

    move_cursor_left_n_columns(line_ctx, line_ctx->cursor_index);
}

static void handle_end_key(editline_st * const editline_ctx)
{
    line_context_st * const line_ctx = &editline_ctx->line_context;

    move_cursor_right_n_columns(line_ctx, line_ctx->line_length - line_ctx->cursor_index);
}

static void handle_insert_key(editline_st * const editline_ctx)
{
    editline_ctx->insert_mode = !editline_ctx->insert_mode;
    /* FIXME - Change the cursor shape according to current mode. */
}

static void handle_backspace(editline_st * const editline_ctx)
{
    line_context_st * const line_ctx = &editline_ctx->line_context;

    delete_char_to_the_left(line_ctx);
}

static void handle_delete(editline_st * const editline_ctx)
{
    line_context_st * const line_ctx = &editline_ctx->line_context;

    delete_char(line_ctx, true);
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

static void handle_up_arrow(editline_st * const editline_ctx)
{
    history_st * const history = editline_ctx->history;
    line_context_st * const line_ctx = &editline_ctx->line_context;
    char const * historic_line;

    if (history_currently_at_most_recent(editline_ctx->history))
    {
        /* save the current line in case the user returns to the top 
         * of the history 
         */
        save_current_line(line_ctx, &editline_ctx->saved_line);

    }
    historic_line = history_get_older_entry(history);

    if (historic_line != NULL)
    {
        replace_line(line_ctx, historic_line);
    }

}

static void handle_down_arrow(editline_st * const editline_ctx)
{
    history_st * const history = editline_ctx->history;
    char const * replacement_line;

    replacement_line = history_get_newer_entry(history);
    if (replacement_line == NULL)
    {
        if (history_currently_at_most_recent(history))
        {
            if (editline_ctx->saved_line != NULL)
            {
                line_context_st * line_ctx = &editline_ctx->line_context;

                /* If the current line matched what we had saved and we still 
                 * replaced it, we'd get the side-effect that the current 
                 * cursor editing poistion would jump to the end of the line. 
                 */
                if (strcmp(line_ctx->buffer, editline_ctx->saved_line) != 0)
                {
                    replacement_line = editline_ctx->saved_line;
                }
            }
        }
    }

    if (replacement_line != NULL)
    {
        line_context_st * line_ctx = &editline_ctx->line_context;

        replace_line(line_ctx, replacement_line);
        if (replacement_line == editline_ctx->saved_line)
        {
            free_saved_line(&editline_ctx->saved_line);
        }
    }
}

static editline_status_t handle_escaped_char(editline_st * const editline_ctx)
{
    editline_status_t status = editline_status_continue;
    int escaped_char;
    tty_get_result_t tty_get_result;

    tty_get_result = tty_get(editline_ctx->in_fd, editline_ctx->maximum_seconds_to_wait_for_char, &escaped_char);
    switch (tty_get_result)
    {
        case tty_get_result_eof:
            status = editline_status_eof;
            goto done; 

        case tty_get_result_timeout:
            status = editline_status_timed_out;
            goto done;
        case tty_get_result_ok:
            break;
    }

    if (escaped_char == '[' || escaped_char == 'O')
    {
        int escape_command_char;

        tty_get_result = tty_get(editline_ctx->in_fd, editline_ctx->maximum_seconds_to_wait_for_char, &escape_command_char);
        switch (tty_get_result)
        {
            case tty_get_result_eof:
                status = editline_status_eof;
                goto done;

            case tty_get_result_timeout:
                status = editline_status_timed_out;
                goto done;
            case tty_get_result_ok:
                break;
        }

        switch (escape_command_char)
        {
            case '1':
                tty_get(editline_ctx->in_fd, editline_ctx->maximum_seconds_to_wait_for_char, NULL);  /* this sequence includes a trailing '~' char */
                handle_home_key(editline_ctx);
                break;
            case '2':
                tty_get(editline_ctx->in_fd, editline_ctx->maximum_seconds_to_wait_for_char, NULL);  /* this sequence includes a trailing '~' char */
                handle_insert_key(editline_ctx);
                break;
            case '3':
                tty_get(editline_ctx->in_fd, editline_ctx->maximum_seconds_to_wait_for_char, NULL);  /* this sequence includes a trailing '~' char */
                handle_delete(editline_ctx);
                break;
            case '4':
                tty_get(editline_ctx->in_fd, editline_ctx->maximum_seconds_to_wait_for_char, NULL);  /* this sequence includes a trailing '~' char */
                handle_end_key(editline_ctx);
                break;
            case '5':
                tty_get(editline_ctx->in_fd, editline_ctx->maximum_seconds_to_wait_for_char, NULL);  /* this sequence includes a trailing '~' char */
                // TODO: handle_page_up(editline_ctx);
                break;
            case '6':
                tty_get(editline_ctx->in_fd, editline_ctx->maximum_seconds_to_wait_for_char, NULL);  /* this sequence includes a trailing '~' char */
                // TODO: handle_page_down(editline_ctx);
                break;
            case 'A':
                handle_up_arrow(editline_ctx);
                break;
            case 'B':
                handle_down_arrow(editline_ctx);
                break;
            case 'C':
                handle_right_arrow(editline_ctx);
                break;
            case 'D':
                handle_left_arrow(editline_ctx);
                break;
            case 'E':   /* 5 on the numeric keypad */
                break;
            case 'F':
                handle_end_key(editline_ctx);
                break;
            case 'H':
                handle_home_key(editline_ctx);
                break;
            default:
                break;
        }
    }

done:
    return status;
}

static editline_status_t handle_enter(editline_st const * const editline_ctx)
{
    tty_puts(editline_ctx->out_fd, newline_str);

    return editline_status_done;
}

static void handle_tab(editline_st * const editline_ctx)
{
    do_word_completion(editline_ctx);
}

static editline_status_t handle_control_char(editline_st * const editline_ctx, int const ch)
{
    editline_status_t status = editline_status_continue;

    switch (ch)
    {
        case '\t': /* TAB */
            handle_tab(editline_ctx); 
            break;
        case '\n': /* ENTER */
            status = handle_enter(editline_ctx);
            break;
        case CTL('C'):
            status = editline_status_ctrl_c;
            break;
        default:  /* silently ignore */
            break;
    }                 

    return status;
}

static void handle_regular_char(editline_st * const editline_ctx, int const ch, bool const update_terminal)
{
    line_context_st * const line_ctx = &editline_ctx->line_context;    

    write_char(line_ctx, ch, editline_ctx->insert_mode, update_terminal);
}

static editline_status_t get_new_input_from_terminal(editline_st * const editline_ctx)
{
    int c;
    editline_status_t status;
    tty_get_result_t tty_get_result;

    tty_get_result = tty_get(editline_ctx->in_fd, editline_ctx->maximum_seconds_to_wait_for_char, &c);
    switch (tty_get_result)
    {
        case tty_get_result_eof:
            status = editline_status_eof;
            goto done;

        case tty_get_result_timeout:
            status = editline_status_timed_out;
            goto done;
        case tty_get_result_ok:
            break;
    }

    switch (c)
    {
        case ESC:
            status = handle_escaped_char(editline_ctx);
            break;
        case BACKSPACE:
            handle_backspace(editline_ctx);
            status = editline_status_continue;
            break;
        default:
            if (ISCTL(c))
            {
                status = handle_control_char(editline_ctx, c);
            }
            else
            {
                handle_regular_char(editline_ctx, c, true);
                status = editline_status_continue;
            }
            break;
    }

done:
    return status;
}

static editline_status_t get_new_input_from_file(editline_st * const editline_ctx)
{
    int c;
    editline_status_t status;
    tty_get_result_t tty_get_result;

    tty_get_result = tty_get(editline_ctx->in_fd, editline_ctx->maximum_seconds_to_wait_for_char, &c);
    switch (tty_get_result)
    {
        case tty_get_result_eof:
            status = editline_status_eof;
            goto done;
        case tty_get_result_timeout:
            status = editline_status_timed_out;
            goto done;
        case tty_get_result_ok:
            break;
    }

    switch (c)
    {
        case '\n':
            status = editline_status_done;
            break;
        default:
            handle_regular_char(editline_ctx, c, false);
            status = editline_status_continue;
            break;
    }

done:
    return status;
}

static editline_status_t get_new_input(editline_st * const editline_ctx)
{
    editline_status_t status;

    if (editline_ctx->is_a_terminal)
    {
        status = get_new_input_from_terminal(editline_ctx);
    }
    else
    {
        status = get_new_input_from_file(editline_ctx);
    }

    return status;
}

static editline_status_t edit_input(editline_st * const editline_ctx)
{
    editline_status_t status = editline_status_continue;

    if (editline_ctx->is_a_terminal)
    {
        tty_puts(editline_ctx->out_fd, editline_ctx->prompt);
    }

    do
    {
        status = get_new_input(editline_ctx);
    }
    while (status == editline_status_continue);

    return status;
}

static bool readline_init(editline_st * const editline_ctx)
{
    bool readline_prepared;
    line_context_st * const line_ctx = &editline_ctx->line_context;

    if (!line_buffer_init(line_ctx,
                          INITIAL_LINE_BUFFER_SIZE,
                          LINE_BUFFER_SIZE_INCREMENT,
                          editline_ctx->out_fd))
    {
        readline_prepared = false;
        goto done;
    }

    if (editline_ctx->is_a_terminal)
    {
        editline_ctx->terminal_width = get_terminal_width(editline_ctx->out_fd);

        history_reset(editline_ctx->history);

        prepare_terminal(&editline_ctx->previous_terminal_settings);
    }

    free_saved_line(&editline_ctx->saved_line);

    readline_prepared = true;

done:
    return readline_prepared;
}

static void readline_cleanup(editline_st * const editline_ctx)
{
    line_context_st * const line_ctx = &editline_ctx->line_context;

    if (editline_ctx->is_a_terminal)
    {
        restore_terminal(&editline_ctx->previous_terminal_settings);
    }

    line_buffer_teardown(line_ctx);
    free_saved_line(&editline_ctx->saved_line);
}

editline_result_t readline(editline_st * const editline_ctx, unsigned int const timeout_seconds, char const * const prompt, char * * const line)
{
    editline_status_t editline_status;
    editline_result_t editline_result;
    line_context_st * const line_ctx = &editline_ctx->line_context;

    editline_ctx->maximum_seconds_to_wait_for_char = timeout_seconds;
    editline_ctx->prompt = prompt; 

    if (!readline_init(editline_ctx))
    {
        editline_result = editline_result_error;
        goto done;
    }

    editline_status = edit_input(editline_ctx);

    switch(editline_status)
    {
        case editline_status_done:
        {
            if (editline_ctx->is_a_terminal)
            {
                history_st * history = editline_ctx->history;

                history_add(history, line_ctx->buffer);
            }
            *line = line_ctx->buffer;
            line_ctx->buffer = NULL;
            editline_result = editline_result_success;
            break;
        }
        case editline_status_continue:  /* Shouldn't happen, but if it does, call it an error. */
            /* drop through */
        case editline_status_error:
            editline_result = editline_result_error;
            break;
        case editline_status_eof:
            editline_result = editline_result_eof;
            break;
        case editline_status_ctrl_c:
            editline_result = editline_result_ctrl_c;
            break;
        case editline_status_timed_out:
            editline_result = editline_result_timed_out;
            break;
    }

    readline_cleanup(editline_ctx);

done:

    if (editline_result != editline_result_success)
    {
        *line = NULL;
    }

    return editline_result;
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
editline_result_t readline_args(editline_st * const editline_ctx, 
                                unsigned int const timeout_seconds,
                                char const * const prompt, 
                                size_t * const argc, 
                                char const * * * argv)
{
    editline_result_t result;
    char * line;
    tokens_st * tokens = NULL;
    args_st * args;

    result = readline(editline_ctx, timeout_seconds, prompt, &line);
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

