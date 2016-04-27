#include "line_context.h"
#include "terminal.h"
#include "screen.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define MIN(x,y) ((x) < (y) ? (x) : (y))

/* Write a char at the current cursor position to the 
 * terminal. 
 */
static void terminal_write_char(line_context_st * const line_ctx, int const ch, bool const insert_mode)
{
    char const char_to_write = line_ctx->mask_character != '\0' ? line_ctx->mask_character : ch;

    screen_put(line_ctx, char_to_write);
    /* if in insert mode, any chars after the one just written
     * will need to be written out.
     */
    if (insert_mode)
    {
        size_t const trailing_length = line_ctx->line_length - line_ctx->cursor_index;

        if (trailing_length > 0)
        {
            size_t const original_screen_cursor_row = line_ctx->terminal_cursor.row;
            size_t const original_screen_cursor_index = line_ctx->terminal_cursor.column;
            size_t new_screen_cursor_row;
            size_t new_screen_cursor_index; 

            screen_puts(line_ctx, &line_ctx->buffer[line_ctx->cursor_index], line_ctx->mask_character);

            /* Move the cursor back to where it was before we output the 
             * trailing chars. 
             */
            new_screen_cursor_row = line_ctx->terminal_cursor.row;
            new_screen_cursor_index = line_ctx->terminal_cursor.column;

            if (new_screen_cursor_row > original_screen_cursor_row)
            {
                size_t const rows_to_move_up = new_screen_cursor_row - original_screen_cursor_row;

                move_physical_cursor_up(line_ctx->terminal_cursor.terminal_fd, rows_to_move_up);
                line_ctx->terminal_cursor.row -= rows_to_move_up;
            }
            if (new_screen_cursor_index != original_screen_cursor_index)
            {
                if (new_screen_cursor_index > original_screen_cursor_index)
                {
                    size_t const columns_to_move = new_screen_cursor_index - original_screen_cursor_index;

                    move_physical_cursor_left(line_ctx->terminal_cursor.terminal_fd,
                                              columns_to_move);
                    line_ctx->terminal_cursor.column -= columns_to_move;
                }
                else
                {
                    size_t const columns_to_move = original_screen_cursor_index - new_screen_cursor_index;

                    move_physical_cursor_right(line_ctx->terminal_cursor.terminal_fd,
                                               columns_to_move);
                    line_ctx->terminal_cursor.column += columns_to_move;
                }
            }
        }
    }
}

static bool check_line_buffer_size(line_context_st * const line_ctx, size_t const space_required)
{
    bool buffer_size_ok;
    size_t new_buffer_size;

    /* Check that there is room for 'characters_to_add' 
     * characters. 
     */
    if (line_ctx->maximum_line_length > 0 && line_ctx->line_length == line_ctx->maximum_line_length)
    {
        buffer_size_ok = false;
        goto done;
    }
    if (line_ctx->line_length + 1 + space_required <= line_ctx->buffer_size)
    {
        buffer_size_ok = true;
        goto done;
    }

    new_buffer_size = line_ctx->line_length + 1 + space_required + line_ctx->size_increment;

    line_ctx->buffer = realloc(line_ctx->buffer, new_buffer_size);
    if (line_ctx->buffer == NULL)
    {
        line_ctx->buffer_size = 0;
        buffer_size_ok = false;
        goto done; 
    }
    line_ctx->buffer_size = new_buffer_size;
    buffer_size_ok = true; 

done:
    return buffer_size_ok;
}

static bool line_ctx_write_char(line_context_st * const line_ctx, int const ch, bool const insert_mode)
{
    bool char_was_written;
    size_t const trailing_length = line_ctx->line_length - line_ctx->cursor_index;
    bool const line_length_will_increase = insert_mode || (line_ctx->cursor_index >= line_ctx->line_length);

    if (line_length_will_increase)
    {
        if (!check_line_buffer_size(line_ctx, 1))
        {
            char_was_written = false;
            goto done;
        }
    }

    if (insert_mode)
    {
        if (trailing_length > 0)
        {
            memmove(&line_ctx->buffer[line_ctx->cursor_index + 1], &line_ctx->buffer[line_ctx->cursor_index], trailing_length);
        }
    }

    line_ctx->buffer[line_ctx->cursor_index] = ch;
    line_ctx->cursor_index++;

    if (line_length_will_increase)
    {
        line_ctx->line_length++;
        /* keep the line NUL terminated */
        line_ctx->buffer[line_ctx->line_length] = '\0';
    }
    char_was_written = true;

done:
    return char_was_written;
}

static void delete_physical_line_from_cursor_to_end(line_context_st * const line_ctx)
{
    size_t screen_cursor_row;
    size_t rows_left_to_move = line_ctx->terminal_cursor.column;
    size_t rows_to_move_up;
    size_t columns_to_move_right;

    delete_to_end_of_line(line_ctx->terminal_cursor.terminal_fd);
    /* Must also remove any other lines below this one. */

    for (screen_cursor_row = (line_ctx->terminal_cursor.row + 1);
         screen_cursor_row < line_ctx->terminal_cursor.num_rows;
         screen_cursor_row++)
    {
        move_physical_cursor_down(line_ctx->terminal_cursor.terminal_fd, 1);
        if (rows_left_to_move > 0)
        {
            move_physical_cursor_left(line_ctx->terminal_cursor.terminal_fd, line_ctx->terminal_cursor.column);
            rows_left_to_move = 0;
        }
        delete_to_end_of_line(line_ctx->terminal_cursor.terminal_fd);
    }
    rows_to_move_up = line_ctx->terminal_cursor.num_rows - line_ctx->terminal_cursor.row - 1;
    columns_to_move_right = line_ctx->terminal_cursor.column - rows_left_to_move;

    move_physical_cursor_up(line_ctx->terminal_cursor.terminal_fd, rows_to_move_up);
    move_physical_cursor_right(line_ctx->terminal_cursor.terminal_fd, columns_to_move_right);

}

static void delete_line_from_cursor_to_end(line_context_st * const line_ctx)
{
    line_ctx->line_length = line_ctx->cursor_index;
    line_ctx->buffer[line_ctx->line_length] = '\0'; 

    delete_physical_line_from_cursor_to_end(line_ctx);
}

static void restore_cursor_position(line_context_st * const line_ctx, size_t const original_cursor_position)
{
    if (line_ctx->cursor_index > original_cursor_position)
    {
        move_cursor_left_n_columns(line_ctx, line_ctx->cursor_index - original_cursor_position);
    }
}

void redisplay_line(line_context_st * const line_ctx)
{
    size_t const original_cursor_index = line_ctx->cursor_index;

    tty_put(line_ctx->terminal_fd, '\n');

    terminal_cursor_reset(&line_ctx->terminal_cursor);

    screen_puts(line_ctx, line_ctx->prompt, '\0');

    screen_puts(line_ctx, line_ctx->buffer, line_ctx->mask_character);
    line_ctx->cursor_index = strlen(line_ctx->buffer);
    restore_cursor_position(line_ctx, original_cursor_index);
}

bool line_context_init(line_context_st * const line_context,
                       size_t const initial_size, 
                       size_t const size_increment,
                       size_t const maximum_line_length,
                       int const terminal_fd,
                       size_t const terminal_width,
                       int const mask_character,
                       char const * const prompt)
{
    bool init_ok;

    /* don't allow a zero increment. Default to 1 if specified. */
    line_context->size_increment = size_increment == 0 ? 1 : size_increment;
    line_context->buffer_size = initial_size;
    /* free any old line_buffer */
    free(line_context->buffer);
    line_context->buffer = (char *)malloc(line_context->buffer_size);
    if (line_context->buffer == NULL)
    {
        init_ok = false;
        goto done;
    }
    line_context->terminal_fd = terminal_fd;
    line_context->line_length = 0;
    line_context->buffer[line_context->line_length] = '\0';
    line_context->maximum_line_length = maximum_line_length;

    line_context->cursor_index = 0;
    line_context->terminal_cursor.num_rows = 1;
    line_context->terminal_width = terminal_width;

    line_context->mask_character = mask_character;
    line_context->prompt = prompt;

    terminal_cursor_init(&line_context->terminal_cursor, line_context->terminal_fd, line_context->terminal_width);

    init_ok = true;

done:
    return init_ok;
}

void line_context_teardown(line_context_st * const line_context)
{
    /* free any old line_buffer */
    free(line_context->buffer);
    line_context->buffer = NULL;
}

void move_cursor_right_n_columns(line_context_st * const line_ctx, size_t columns)
{
    size_t columns_to_move = MIN(columns, line_ctx->line_length - line_ctx->cursor_index);

    if (columns_to_move > 0)
    {
        line_ctx->cursor_index += columns_to_move; 

        move_terminal_cursor_right_n_columns(line_ctx, columns_to_move);

    }
}

void move_cursor_left_n_columns(line_context_st * const line_ctx, size_t const columns)
{
    size_t columns_to_move = MIN(columns, line_ctx->cursor_index);

    if (columns_to_move > 0)
    {
        line_ctx->cursor_index -= columns_to_move;

        move_terminal_cursor_left_n_columns(line_ctx, columns_to_move);
    }
}

/*
 * replace_edit_line: 
 * Replace the whole line except for the leading prompt with the
 * replacement. 
*/
void replace_edit_line(line_context_st * const line_ctx, char const * const replacement)
{
    move_cursor_left_n_columns(line_ctx, line_ctx->cursor_index);
    delete_line_from_cursor_to_end(line_ctx);
    write_string(line_ctx, replacement, true, true);
}

void delete_char_to_the_right(line_context_st * const line_ctx, bool const update_display)
{
    if (line_ctx->cursor_index < line_ctx->line_length)
    {
        size_t const trailing_chars = line_ctx->line_length - line_ctx->cursor_index;

        line_ctx->line_length--;
        memmove(&line_ctx->buffer[line_ctx->cursor_index], &line_ctx->buffer[line_ctx->cursor_index + 1], trailing_chars);
        line_ctx->buffer[line_ctx->line_length] = '\0';

        if (update_display)
        {
            screen_puts(line_ctx, &line_ctx->buffer[line_ctx->cursor_index], line_ctx->mask_character);
            /* Remove the remaining char from the end of the line by 
             * replacing it with a space. 
             */
            screen_put(line_ctx, ' ');
            move_terminal_cursor_left_n_columns(line_ctx, trailing_chars);
        }
    }
}

void delete_char_to_the_left(line_context_st * const line_ctx)
{
    /* If we move the cursor to the left one position, this 
     * operation becomes just like deleting the character to the 
     * right. 
     */
    if (line_ctx->cursor_index > 0)
    {
        move_cursor_left_n_columns(line_ctx, 1);
        delete_char_to_the_right(line_ctx, true);
    }
}

/* Write a char at the current cursor position. */
void write_char(line_context_st * const line_ctx, int const ch, bool const insert_mode, bool const update_terminal)
{
    if (line_ctx_write_char(line_ctx, ch, insert_mode))
    {
        if (update_terminal)
        {
            terminal_write_char(line_ctx, ch, insert_mode);
        }
    }
}

/* Write a string at the current cursor position. */
void write_string(line_context_st * const line_ctx, char const * const string, bool insert_mode, bool const update_terminal)
{
    char const * pch = string;

    while (*pch)
    {
        write_char(line_ctx, *pch, insert_mode, update_terminal);
        pch++;
    }
}

static void delete_to_end_of_word(line_context_st * const line_ctx, bool const update_terminal)
{
    while (line_ctx->buffer[line_ctx->cursor_index] != '\0' && !isspace(line_ctx->buffer[line_ctx->cursor_index]))
    {
        delete_char_to_the_right(line_ctx, update_terminal);
    }
}

void complete_word(line_context_st * const line_ctx, char const * const completion, bool const update_terminal)
{
    /* Remove any chars at the end of the word, then write the 
     * supplied completion suffix at the current cursor position 
     */
    delete_to_end_of_word(line_ctx, update_terminal);
    write_string(line_ctx, completion, true, update_terminal);
}

void free_saved_line(char const * * const saved_line)
{
    free((void *) *saved_line);
    *saved_line = NULL;
}

void save_current_line(line_context_st * const line_ctx, char const * * const destination)
{
    /* Free any old string at the destination. */
    free_saved_line(destination);
    *destination = strdup(line_ctx->buffer);
}


