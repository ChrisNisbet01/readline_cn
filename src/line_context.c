#include "line_context.h"
#include "terminal.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define MIN(x,y) ((x) < (y) ? (x) : (y))

/* write a char at the current cursor position. */
static void terminal_write_char(line_context_st * const line_ctx, int const ch, bool const insert_mode)
{
    char const char_to_write = line_ctx->mask_character != '\0' ? line_ctx->mask_character : ch;

    tty_put(line_ctx->terminal_fd, char_to_write);
    /* if in insert mode, any chars after the one just pushed in 
     * will need to be written out.
     */
    if (insert_mode)
    {
        size_t const trailing_length = line_ctx->line_length - line_ctx->cursor_index;

        if (trailing_length > 0)
        {
            tty_puts(line_ctx->terminal_fd, &line_ctx->buffer[line_ctx->cursor_index], line_ctx->mask_character);
            /* Move the cursor back to where it was before we output the 
             * trailing chars. 
             */
            move_physical_cursor_left(line_ctx->terminal_fd, trailing_length);
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

static void line_ctx_write_char(line_context_st * const line_ctx, int const ch, bool const insert_mode)
{
    size_t const trailing_length = line_ctx->line_length - line_ctx->cursor_index;

    if (!check_line_buffer_size(line_ctx, 1))
    {
        goto done;
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

    if (insert_mode || line_ctx->cursor_index > line_ctx->line_length)
    {
        line_ctx->line_length++;
    }

    /* keep the line NUL terminated */
    line_ctx->buffer[line_ctx->line_length] = '\0';

done:
    return;
}

static void delete_line_from_cursor_to_end(line_context_st * const line_ctx)
{
    delete_to_end_of_line(line_ctx->terminal_fd);
    line_ctx->line_length = line_ctx->cursor_index;
    line_ctx->buffer[line_ctx->line_length] = '\0';
}

static void print_current_edit_line(line_context_st * const line_ctx)
{
    tty_puts(line_ctx->terminal_fd, line_ctx->buffer, '\0');
    line_ctx->cursor_index = strlen(line_ctx->buffer);
}

void redisplay_line(line_context_st * const line_ctx, char const * const prompt)
{
    size_t const original_cursor_index = line_ctx->cursor_index;

    tty_put(line_ctx->terminal_fd, '\n');
    tty_puts(line_ctx->terminal_fd, prompt, '\0');
    print_current_edit_line(line_ctx);
    /* Move the cursor back to where it was before we redisplayed 
     * the line. 
     */
    if (line_ctx->cursor_index > original_cursor_index)
    {
        move_cursor_left_n_columns(line_ctx, line_ctx->cursor_index - original_cursor_index);
    }
}

bool line_context_init(line_context_st * const line_context,
                      size_t const initial_size, 
                      size_t const size_increment,
                      int const terminal_fd,
                      int const mask_character)
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
    line_context->line_length = 0;
    line_context->cursor_index = 0;
    line_context->buffer[0] = '\0';

    line_context->terminal_fd = terminal_fd;
    line_context->mask_character = mask_character;

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
        if (move_physical_cursor_right(line_ctx->terminal_fd, columns_to_move))
        {
            line_ctx->cursor_index += columns_to_move;
        }
    }
}

void move_cursor_left_n_columns(line_context_st * const line_ctx, size_t const columns)
{
    size_t columns_to_move = MIN(columns, line_ctx->cursor_index);

    if (columns_to_move > 0)
    {
        if (move_physical_cursor_left(line_ctx->terminal_fd, columns_to_move))
        {
            line_ctx->cursor_index -= columns_to_move;
        }
    }
}

void replace_line(line_context_st * const line_ctx, char const * const replacement)
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
            tty_puts(line_ctx->terminal_fd, &line_ctx->buffer[line_ctx->cursor_index], line_ctx->mask_character);
            /* Remove the remaining char from the end of the line by 
             * replacing it with a space. 
             */
            tty_put(line_ctx->terminal_fd, ' ');
            move_physical_cursor_left(line_ctx->terminal_fd, trailing_chars);
        }
    }
}

void delete_char_to_the_left(line_context_st * const line_ctx)
{
    /* If we move the cursor to the left one position, the 
     * operation becomes just like deleting the character to the 
     * right. 
     */
    if (line_ctx->cursor_index > 0)
    {
        line_ctx->cursor_index--;
        move_physical_cursor_left(line_ctx->terminal_fd, 1);
        delete_char_to_the_right(line_ctx, true);
    }
}

/* Write a char at the current cursor position. */
void write_char(line_context_st * const line_ctx, int const ch, bool const insert_mode, bool const update_terminal)
{
    line_ctx_write_char(line_ctx, ch, insert_mode);

    if (update_terminal)
    {
        terminal_write_char(line_ctx, ch, insert_mode);
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


