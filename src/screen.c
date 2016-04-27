#include "screen.h"
#include "terminal.h"

#define MAX(x,y) ((x) > (y) ? (x) : (y))

void screen_put(line_context_st * const line_ctx, char const ch)
{
    tty_put(line_ctx->terminal_fd, ch);
    line_ctx->screen_cursor_index++;
    if (line_ctx->screen_cursor_index == line_ctx->terminal_width)
    {
        tty_put(line_ctx->terminal_fd, '\n');
        line_ctx->screen_cursor_index = 0;
        line_ctx->screen_cursor_row++;
        line_ctx->num_rows = MAX(line_ctx->num_rows, line_ctx->screen_cursor_row + 1);
    }
}

void screen_puts(line_context_st * const line_ctx, char const * const string, char const mask_character)
{
    char const * p = string;

    while ( *p != '\0')
    {
        char const char_to_put = (mask_character != '\0') ? mask_character : *p;

        screen_put(line_ctx, char_to_put);
        p++;
    }
}

void move_terminal_cursor_right_n_columns(line_context_st * const line_ctx, size_t const columns)
{
    size_t const original_terminal_cursor_index = line_ctx->screen_cursor_index;
    size_t new_terminal_cursor_index;
    size_t rows_to_move;
    size_t const columns_to_move = columns;

    rows_to_move = (line_ctx->screen_cursor_index + columns_to_move) / line_ctx->terminal_width;
    new_terminal_cursor_index = (line_ctx->screen_cursor_index + columns_to_move) % line_ctx->terminal_width;

    line_ctx->screen_cursor_index = new_terminal_cursor_index;
    line_ctx->screen_cursor_row += rows_to_move;

    /* Update the physical cursor row. */
    if (rows_to_move > 0)
    {
        move_physical_cursor_down(line_ctx->terminal_fd, rows_to_move);
    }
    /* Update the physical cursor column */
    if (line_ctx->screen_cursor_index > original_terminal_cursor_index)
    {
        size_t const chars_to_move = line_ctx->screen_cursor_index - original_terminal_cursor_index;

        move_physical_cursor_right(line_ctx->terminal_fd, chars_to_move);
    }
    else if (line_ctx->screen_cursor_index < original_terminal_cursor_index)
    {
        size_t const chars_to_move = original_terminal_cursor_index - line_ctx->screen_cursor_index;

        move_physical_cursor_left(line_ctx->terminal_fd, chars_to_move);
    }
}

void move_terminal_cursor_left_n_columns(line_context_st * const line_ctx, size_t const columns)
{
    size_t const original_screen_cursor_index = line_ctx->screen_cursor_index;
    size_t new_screen_cursor_index;
    size_t rows_to_move;
    size_t columns_to_move = columns;

    /* Update the physical cursor. */
    rows_to_move = columns_to_move / line_ctx->terminal_width;
    columns_to_move %= line_ctx->terminal_width;

    if (line_ctx->screen_cursor_index >= columns_to_move)
    {
        new_screen_cursor_index = line_ctx->screen_cursor_index - columns_to_move;
    }
    else
    {
        new_screen_cursor_index = line_ctx->screen_cursor_index + line_ctx->terminal_width - columns_to_move;
        rows_to_move++;
    }

    line_ctx->screen_cursor_index = new_screen_cursor_index;
    line_ctx->screen_cursor_row -= rows_to_move;

    /* Update the physical cursor row. */
    if (rows_to_move > 0)
    {
        move_physical_cursor_up(line_ctx->terminal_fd, rows_to_move);
    }
    /* Update the physical cursor column */
    if (line_ctx->screen_cursor_index > original_screen_cursor_index)
    {
        size_t const chars_to_move = line_ctx->screen_cursor_index - original_screen_cursor_index;

        move_physical_cursor_right(line_ctx->terminal_fd, chars_to_move);
    }
    else if (line_ctx->screen_cursor_index < original_screen_cursor_index)
    {
        size_t const chars_to_move = original_screen_cursor_index - line_ctx->screen_cursor_index;

        move_physical_cursor_left(line_ctx->terminal_fd, chars_to_move);
    }
}

void terminal_init(terminal_st * const terminal, int const out_fd, size_t const terminal_width)
{
    terminal->out_fd = out_fd;
    terminal->width = terminal_width;
    terminal->num_rows = 1;
    terminal->cursor_row = 0;
    terminal->cursor_column = 0;
}
