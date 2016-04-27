#include "screen.h"
#include "terminal.h"

#define MAX(x,y) ((x) > (y) ? (x) : (y))

void screen_put(line_context_st * const line_ctx, char const ch)
{
    tty_put(line_ctx->terminal_cursor.terminal_fd, ch);
    line_ctx->terminal_cursor.column++;
    if (line_ctx->terminal_cursor.column == line_ctx->terminal_width)
    {
        tty_put(line_ctx->terminal_cursor.terminal_fd, '\n');
        line_ctx->terminal_cursor.column = 0;
        line_ctx->terminal_cursor.row++;
        line_ctx->terminal_cursor.num_rows = MAX(line_ctx->terminal_cursor.num_rows, line_ctx->terminal_cursor.row + 1);
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
    size_t const original_terminal_cursor_index = line_ctx->terminal_cursor.column;
    size_t new_terminal_cursor_column;
    size_t rows_to_move;
    size_t const columns_to_move = columns;

    rows_to_move = (line_ctx->terminal_cursor.column + columns_to_move) / line_ctx->terminal_width;
    new_terminal_cursor_column = (line_ctx->terminal_cursor.column + columns_to_move) % line_ctx->terminal_width;

    line_ctx->terminal_cursor.column = new_terminal_cursor_column;
    line_ctx->terminal_cursor.row += rows_to_move;

    /* Update the physical cursor row. If moving, it can only move 
     * down. 
     */
    if (rows_to_move > 0)
    {
        move_physical_cursor_down(line_ctx->terminal_cursor.terminal_fd, rows_to_move);
    }
    /* Update the physical cursor column */
    if (new_terminal_cursor_column > original_terminal_cursor_index)
    {
        size_t const chars_to_move = new_terminal_cursor_column - original_terminal_cursor_index;

        move_physical_cursor_right(line_ctx->terminal_cursor.terminal_fd, chars_to_move);
    }
    else if (new_terminal_cursor_column < original_terminal_cursor_index)
    {
        size_t const chars_to_move = original_terminal_cursor_index - new_terminal_cursor_column;

        move_physical_cursor_left(line_ctx->terminal_cursor.terminal_fd, chars_to_move);
    }
}

void move_terminal_cursor_left_n_columns(line_context_st * const line_ctx, size_t const columns)
{
    size_t const original_screen_cursor_column = line_ctx->terminal_cursor.column;
    size_t new_screen_cursor_index;
    size_t rows_to_move;
    size_t columns_to_move = columns;

    /* Update the physical cursor. */
    rows_to_move = columns_to_move / line_ctx->terminal_width;
    columns_to_move %= line_ctx->terminal_width;

    if (line_ctx->terminal_cursor.column >= columns_to_move)
    {
        new_screen_cursor_index = line_ctx->terminal_cursor.column - columns_to_move;
    }
    else
    {
        new_screen_cursor_index = line_ctx->terminal_cursor.column + line_ctx->terminal_width - columns_to_move;
        rows_to_move++;
    }

    line_ctx->terminal_cursor.column = new_screen_cursor_index;
    line_ctx->terminal_cursor.row -= rows_to_move;

    /* Update the physical cursor row. If moving, it can only move 
     * up. 
     */
    if (rows_to_move > 0)
    {
        move_physical_cursor_up(line_ctx->terminal_cursor.terminal_fd, rows_to_move);
    }
    /* Update the physical cursor column */
    if (new_screen_cursor_index > original_screen_cursor_column)
    {
        size_t const chars_to_move = new_screen_cursor_index - original_screen_cursor_column;

        move_physical_cursor_right(line_ctx->terminal_cursor.terminal_fd, chars_to_move);
    }
    else if (new_screen_cursor_index < original_screen_cursor_column)
    {
        size_t const chars_to_move = original_screen_cursor_column - new_screen_cursor_index;

        move_physical_cursor_left(line_ctx->terminal_cursor.terminal_fd, chars_to_move);
    }
}

void terminal_cursor_reset(terminal_cursor_st * const terminal_cursor)
{
    terminal_cursor->row = 0;
    terminal_cursor->column = 0;
}

void terminal_cursor_init(terminal_cursor_st * const terminal_cursor, int const terminal_fd, size_t const terminal_width)
{
    terminal_cursor->terminal_fd = terminal_fd;
    terminal_cursor->terminal_width = terminal_width;
    terminal_cursor->num_rows = 1;
    terminal_cursor_reset(terminal_cursor);
}
