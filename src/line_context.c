#include "line_context.h"
#include "terminal.h"
#include "terminal_cursor.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

static void restore_cursor_position(line_context_st * const line_ctx, size_t const original_cursor_position);

/* Write a char at the current cursor position to the 
 * terminal. If inserting the character, write out all 
 * subsequent characters to keep the display up to date. 
 */
static void terminal_write_char(line_context_st * const line_ctx, int const ch, bool const insert_mode)
{
    char const char_to_write = line_ctx->mask_character != '\0' ? line_ctx->mask_character : ch;
    terminal_cursor_st * const terminal_cursor = &line_ctx->terminal_cursor;

    terminal_put(terminal_cursor, char_to_write);
    /* if in insert mode, any chars after the one just written
     * will need to be written out.
     */
    if (insert_mode)
    {
        size_t const trailing_length = line_ctx->line_length - line_ctx->cursor_index;

        if (trailing_length > 0)
        {
            size_t const original_cursor_index = line_ctx->cursor_index;

            terminal_puts(terminal_cursor, &line_ctx->buffer[line_ctx->cursor_index], line_ctx->mask_character);
            /* Update the current edit position to match the physical 
             * cursor position. 
             */
            line_ctx->cursor_index = strlen(line_ctx->buffer); 
            /* And now restore edit position and cursor back to the original
             * editing location. 
             */
            restore_cursor_position(line_ctx, original_cursor_index);
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

void delete_from_cursor_to_start(line_context_st * const line_ctx)
{
    delete_chars_to_the_left(line_ctx, line_ctx->cursor_index);
}

void delete_from_cursor_to_end(line_context_st * const line_ctx)
{
    line_ctx->line_length = line_ctx->cursor_index;
    line_ctx->buffer[line_ctx->line_length] = '\0'; 

    terminal_delete_line_from_cursor_to_end(&line_ctx->terminal_cursor);
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
    terminal_cursor_st * const terminal_cursor = &line_ctx->terminal_cursor; 

    tty_put(line_ctx->terminal_fd, '\n');

    terminal_cursor_reset(terminal_cursor);

    terminal_puts(terminal_cursor, line_ctx->prompt, '\0');

    terminal_puts(terminal_cursor, line_ctx->buffer, line_ctx->mask_character);
    /* The terminal cursor will now be at the end of the line, so 
     * update the editing position to match. 
     */
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

        terminal_move_cursor_right_n_columns(&line_ctx->terminal_cursor, columns_to_move);

    }
}

void move_cursor_left_n_columns(line_context_st * const line_ctx, size_t const columns)
{
    size_t columns_to_move = MIN(columns, line_ctx->cursor_index);

    if (columns_to_move > 0)
    {
        line_ctx->cursor_index -= columns_to_move;

        terminal_move_cursor_left_n_columns(&line_ctx->terminal_cursor, columns_to_move);
    }
}

static void delete_edit_line(line_context_st * const line_ctx)
{
    move_cursor_left_n_columns(line_ctx, line_ctx->cursor_index);
    delete_from_cursor_to_end(line_ctx);
}

/*
 * replace_edit_line: 
 * Replace the whole line except for the leading prompt with the
 * replacement. 
*/
void replace_edit_line(line_context_st * const line_ctx, char const * const replacement)
{
    delete_edit_line(line_ctx);
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
            terminal_cursor_st * const terminal_cursor = &line_ctx->terminal_cursor;

            terminal_puts(terminal_cursor, &line_ctx->buffer[line_ctx->cursor_index], line_ctx->mask_character);
            /* Remove the remaining char from the end of the line by 
             * replacing it with a space. 
             */
            terminal_put(terminal_cursor, ' ');
            /* Move the terminal cursor back to match the editing 
             * position. 
             */
            terminal_move_cursor_left_n_columns(terminal_cursor, trailing_chars);
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

void delete_chars_to_the_right(line_context_st * const line_ctx, size_t const chars_to_delete)
{
    size_t count;

    for (count = 0; count < chars_to_delete; count++)
    {
        delete_char_to_the_right(line_ctx, true);
    }
}

void delete_chars_to_the_left(line_context_st * const line_ctx, size_t const chars_to_delete)
{
    move_cursor_left_n_columns(line_ctx, chars_to_delete);
    delete_chars_to_the_right(line_ctx, chars_to_delete);
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
    /* TODO - Use field separators rather than isspace? Use 
     * isalnum() instead of !isspace()? 
     */
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

/* Transpose two characters at the cursor position. */
void transpose_characters(line_context_st * const line_ctx)
{
    size_t columns_to_move_left;
    char first_char;
    char second_char;

    if (line_ctx->line_length < 2)
    {
        goto done;
    }
    if (line_ctx->cursor_index == 0)
    {
        goto done;
    }
    if (line_ctx->cursor_index == line_ctx->line_length)
    {
        first_char = line_ctx->buffer[line_ctx->cursor_index - 2];
        second_char = line_ctx->buffer[line_ctx->cursor_index - 1];
        columns_to_move_left = 2;
    }
    else
    {
        first_char = line_ctx->buffer[line_ctx->cursor_index - 1];
        second_char = line_ctx->buffer[line_ctx->cursor_index];
        columns_to_move_left = 1;
    }

    move_cursor_left_n_columns(line_ctx, columns_to_move_left);
    write_char(line_ctx, second_char, false, true);
    write_char(line_ctx, first_char, false, true);

done:
    return;
}

static bool is_word_separator(char const ch)
{
    /* XXX - Also check user supplied field separators? */
    return isalnum((int)ch) == 0;
}

size_t get_index_of_start_of_previous_word(line_context_st * const line_ctx)
{
    size_t index;

    if (line_ctx->cursor_index > 0)
    {
        /* Move left passed word separators. */
        for (index = line_ctx->cursor_index - 1; index != 0; index--)
        {
            if (!is_word_separator(line_ctx->buffer[index]))
            {
                break;
            }
        }
        /* Skip left until the next character to the left is a word 
         * separator. 
         */
        for (; index > 0; index--)
        {
            if (is_word_separator(line_ctx->buffer[index - 1]))
            {
                break;
            }
        }
    }
    else
    {
        index = 0;
    }

    return index;
}

size_t get_index_of_end_of_next_word(line_context_st * const line_ctx)
{
    size_t index;

    if (line_ctx->cursor_index < line_ctx->line_length)
    {
        /* Move right passed word separators. */
        for (index = line_ctx->cursor_index + 1; index < line_ctx->line_length; index++)
        {
            if (!is_word_separator(line_ctx->buffer[index]))
            {
                break;
            }
        }
        /* Move right until we hit a word separator. 
         */
        for (; index < line_ctx->line_length; index++)
        {
            if (is_word_separator(line_ctx->buffer[index]))
            {
                break;
            }
        }
    }
    else
    {
        index = line_ctx->line_length;
    }

    return index;
}

void move_left_to_beginning_of_word(line_context_st * const line_ctx)
{
    size_t const index = get_index_of_start_of_previous_word(line_ctx);

    if (index != line_ctx->cursor_index)
    {
        move_cursor_left_n_columns(line_ctx, line_ctx->cursor_index - index);
    }
}

void move_right_to_end_of_word(line_context_st * const line_ctx)
{
    size_t const index = get_index_of_end_of_next_word(line_ctx);

    if (index != line_ctx->cursor_index)
    {
        move_cursor_right_n_columns(line_ctx, index - line_ctx->cursor_index);
    }
}

void delete_previous_word(line_context_st * const line_ctx)
{
    size_t const index = get_index_of_start_of_previous_word(line_ctx);

    if (index < line_ctx->cursor_index)
    {
        delete_chars_to_the_left(line_ctx, line_ctx->cursor_index - index);
    }
}

void delete_to_next_word(line_context_st * const line_ctx)
{
    size_t const index = get_index_of_end_of_next_word(line_ctx);

    if (index > line_ctx->cursor_index)
    {
        delete_chars_to_the_right(line_ctx, index - line_ctx->cursor_index);
    }
}


