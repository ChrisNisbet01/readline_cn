#include "handlers.h"
#include "word_completion.h"
#include "terminal.h"
#include "read_char.h"
#include "help.h"

#include <string.h>

static readline_status_t handle_enter(readline_st const * const readline_ctx)
{
    tty_put(readline_ctx->out_fd, '\n');

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

readline_status_t handle_control_char(readline_st * const readline_ctx, int const ch)
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

void handle_backspace(readline_st * const readline_ctx)
{
    line_context_st * const line_ctx = &readline_ctx->line_context;

    delete_char_to_the_left(line_ctx);
}

static void handle_delete(readline_st * const readline_ctx)
{
    line_context_st * const line_ctx = &readline_ctx->line_context;

    delete_char_to_the_right(line_ctx, true);
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
        replace_edit_line(line_ctx, historic_line);
    }

}

static void handle_shift_up(readline_st * const readline_ctx)
{
    line_context_st * const line_ctx = &readline_ctx->line_context;

    if (line_ctx->screen_cursor_row > 0)
    {
        size_t chars_moved;

        line_ctx->screen_cursor_row--;

        move_physical_cursor_up(readline_ctx->out_fd, 1);

        chars_moved = line_ctx->terminal_width;
        if (line_ctx->screen_cursor_row == 0)
        {
            size_t const prompt_width = strlen(line_ctx->prompt);

            if (line_ctx->screen_cursor_index < prompt_width)
            {
                size_t const chars_to_move_right = prompt_width - line_ctx->screen_cursor_index;

                move_physical_cursor_right(line_ctx->terminal_fd, chars_to_move_right);
                line_ctx->screen_cursor_index += chars_to_move_right;
                chars_moved -= chars_to_move_right;
            }
        }
        line_ctx->cursor_index -= chars_moved;
        fprintf(stderr, "chars moved %zd cursor index %zd\n", chars_moved, line_ctx->cursor_index);
    }
}

static void handle_shift_down(readline_st * const readline_ctx)
{
    line_context_st * const line_ctx = &readline_ctx->line_context;

    if (line_ctx->screen_cursor_row < line_ctx->num_rows - 1)
    {
        size_t chars_moved;

        line_ctx->screen_cursor_row++;

        move_physical_cursor_down(readline_ctx->out_fd, 1);

        chars_moved = line_ctx->terminal_width;
        line_ctx->cursor_index += chars_moved;
        if (line_ctx->cursor_index > line_ctx->line_length)
        {
            size_t const chars_to_move_left = line_ctx->cursor_index - line_ctx->line_length;

            move_physical_cursor_left(line_ctx->terminal_fd, chars_to_move_left);
            line_ctx->screen_cursor_index -= chars_to_move_left;
            line_ctx->cursor_index = line_ctx->line_length;
        }
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
                 * cursor editing position would jump to the end of the line. 
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

        replace_edit_line(line_ctx, replacement_line);
        if (replacement_line == readline_ctx->saved_line)
        {
            free_saved_line(&readline_ctx->saved_line);
        }
    }
}

static readline_status_t handle_escape_o(readline_st * const readline_ctx)
{
    readline_status_t status;
    int escape_command_char;

    escape_command_char = read_char_from_input_descriptor(readline_ctx->in_fd,
                                                          readline_ctx->maximum_seconds_to_wait_for_char,
                                                          &status);
    if (status != readline_status_continue)
    {
        goto done;
    }

    switch (escape_command_char)
    {
        case 'F':
            handle_end_key(readline_ctx);
            break;
        case 'H':
            handle_home_key(readline_ctx);
            break;
        default:
            /* Silently ignore everything else. */
            break;
    }

done:
    return status;
}

static void handle_escape_left_bracket_1(readline_st * const readline_ctx)
{
    int ch;

    ch = '\0';
    tty_get(readline_ctx->in_fd, readline_ctx->maximum_seconds_to_wait_for_char, &ch);
    switch (ch)
    {
        case '~':
            handle_home_key(readline_ctx);
            break;
        case ';':
        {
            ch = '\0';
            tty_get(readline_ctx->in_fd, readline_ctx->maximum_seconds_to_wait_for_char, &ch);
            switch (ch)
            {
                case '2':
                    ch = '\0';
                    tty_get(readline_ctx->in_fd, readline_ctx->maximum_seconds_to_wait_for_char, &ch);
                    switch (ch)
                    {
                        case 'A':
                            /* Shift + up arrow. */
                            handle_shift_up(readline_ctx);
                            break;
                        case 'B':
                            /* Shift + down arrow. */
                            handle_shift_down(readline_ctx);
                            break;
                        case 'C':
                            /* Shift + right arrow. */
                            break;
                        case 'D':
                            /* Shift + left arrow. */
                            break;
                    }
                    break;
            }
            break;
        }
    }
}

static readline_status_t handle_escape_left_bracket(readline_st * const readline_ctx)
{
    readline_status_t status;
    int escape_command_char;

    escape_command_char = read_char_from_input_descriptor(readline_ctx->in_fd,
                                                          readline_ctx->maximum_seconds_to_wait_for_char,
                                                          &status);
    if (status != readline_status_continue)
    {
        goto done;
    }

    switch (escape_command_char)
    {
        case '1':
            handle_escape_left_bracket_1(readline_ctx);
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
        default:
            break;
    }

done:
    return status;
}

readline_status_t handle_escaped_char(readline_st * const readline_ctx)
{
    readline_status_t status;
    int escaped_char;

    escaped_char = read_char_from_input_descriptor(readline_ctx->in_fd,
                                                   readline_ctx->maximum_seconds_to_wait_for_char,
                                                   &status);
    if (status != readline_status_continue)
    {
        goto done;
    }
    if (escaped_char == 'O')
    {
        status = handle_escape_o(readline_ctx);
    }
    else if (escaped_char == '[')
    {
        status = handle_escape_left_bracket(readline_ctx);
    }
    else
    {
        /* Silently ignore other characters. */
    }

done:
    return status;
}

void handle_regular_char(readline_st * const readline_ctx, int const ch, bool const update_terminal)
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


