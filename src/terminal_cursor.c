/* Copyright (C) Chris Nisbet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly 
 * prohibited. Proprietary and confidential. Written by Chris 
 * Nisbet <nisbet@ihug.co.nz>, April 2016.
 */

#include "terminal_cursor.h"
#include "terminal.h"
#include "utils.h"

/* Write a character to the terminal, keeping note of which row 
 * and column the cursor is on. 
 */
void terminal_put(terminal_cursor_st * const terminal_cursor, 
                  char const ch, 
                  int const terminal_fd,
                  size_t const terminal_width)
{
    tty_put(terminal_fd, ch);
    terminal_cursor->column++;

    if (terminal_cursor->column == terminal_width)
    {
        tty_put(terminal_fd, '\n');
        /* The cursor will now be at the start of the next line, so 
         * update our variables to match. 
         */
        terminal_cursor->column = 0;
        terminal_cursor->row++;
        terminal_cursor->num_rows = MAX(terminal_cursor->num_rows, terminal_cursor->row + 1);
        /* Note that num_rows will never decrease if (say) a long line 
         * is first entered, then replaced with a shorter one by 
         * deleting characters. I don't think this is a big deal. 
         */
    }
}

void terminal_puts(terminal_cursor_st * const terminal_cursor, 
                   char const * const string, 
                   char const mask_character,
                   int const terminal_fd,
                   size_t const terminal_width)
{
    char const * p = string;

    while (*p != '\0')
    {
        char const char_to_put = (mask_character != '\0') ? mask_character : *p;

        terminal_put(terminal_cursor, 
                     char_to_put, 
                     terminal_fd,
                     terminal_width);
        p++;
    }
}

void terminal_move_cursor_right_n_columns(terminal_cursor_st * const terminal_cursor, 
                                          size_t const columns,
                                          int const terminal_fd,
                                          size_t const terminal_width)
{
    size_t const original_terminal_cursor_index = terminal_cursor->column;
    size_t new_terminal_cursor_column;
    size_t rows_to_move;
    size_t const columns_to_move = columns;

    rows_to_move = (terminal_cursor->column + columns_to_move) / terminal_width;
    new_terminal_cursor_column = (terminal_cursor->column + columns_to_move) % terminal_width;

    terminal_cursor->column = new_terminal_cursor_column;
    terminal_cursor->row += rows_to_move;

    /* Update the physical cursor row. If moving, it can only move 
     * down. 
     */
    if (rows_to_move > 0)
    {
        terminal_move_physical_cursor_down(terminal_fd, rows_to_move);
    }
    /* Update the physical cursor column */
    if (new_terminal_cursor_column > original_terminal_cursor_index)
    {
        size_t const chars_to_move = new_terminal_cursor_column - original_terminal_cursor_index;

        terminal_move_physical_cursor_right(terminal_fd, chars_to_move);
    }
    else if (new_terminal_cursor_column < original_terminal_cursor_index)
    {
        size_t const chars_to_move = original_terminal_cursor_index - new_terminal_cursor_column;

        terminal_move_physical_cursor_left(terminal_fd, chars_to_move);
    }
}

void terminal_move_cursor_left_n_columns(terminal_cursor_st * const terminal_cursor, 
                                         size_t const columns,
                                         int const terminal_fd,
                                         size_t const terminal_width)
{
    size_t const original_screen_cursor_column = terminal_cursor->column;
    size_t new_screen_cursor_index;
    size_t rows_to_move;
    size_t columns_to_move = columns;

    /* Update the physical cursor. */
    rows_to_move = columns_to_move / terminal_width;
    columns_to_move %= terminal_width;

    if (terminal_cursor->column >= columns_to_move)
    {
        new_screen_cursor_index = terminal_cursor->column - columns_to_move;
    }
    else
    {
        new_screen_cursor_index = terminal_cursor->column + terminal_width - columns_to_move;
        rows_to_move++;
    }

    terminal_cursor->column = new_screen_cursor_index;
    terminal_cursor->row -= rows_to_move;

    /* Update the physical cursor row. If moving, it can only move 
     * up. 
     */
    if (rows_to_move > 0)
    {
        terminal_move_physical_cursor_up(terminal_fd, rows_to_move);
    }
    /* Update the physical cursor column */
    if (new_screen_cursor_index > original_screen_cursor_column)
    {
        size_t const chars_to_move = new_screen_cursor_index - original_screen_cursor_column;

        terminal_move_physical_cursor_right(terminal_fd, chars_to_move);
    }
    else if (new_screen_cursor_index < original_screen_cursor_column)
    {
        size_t const chars_to_move = original_screen_cursor_column - new_screen_cursor_index;

        terminal_move_physical_cursor_left(terminal_fd, chars_to_move);
    }
}

void terminal_delete_line_from_cursor_to_end(terminal_cursor_st * const terminal_cursor, int const terminal_fd)
{
    size_t screen_cursor_row;
    size_t rows_to_beginning_of_line = terminal_cursor->column;
    size_t rows_to_move_up;
    size_t columns_to_move_right;

    terminal_delete_to_end_of_line(terminal_fd);

    /* Must also remove any other lines below this one. */
    for (screen_cursor_row = (terminal_cursor->row + 1);
         screen_cursor_row < terminal_cursor->num_rows;
         screen_cursor_row++)
    {
        terminal_move_physical_cursor_down(terminal_fd, 1);
        if (rows_to_beginning_of_line > 0)
        {
            terminal_move_physical_cursor_left(terminal_fd, rows_to_beginning_of_line);
            rows_to_beginning_of_line = 0;
        }
        terminal_delete_to_end_of_line(terminal_fd);
    }
    rows_to_move_up = terminal_cursor->num_rows - terminal_cursor->row - 1;
    columns_to_move_right = terminal_cursor->column - rows_to_beginning_of_line;

    terminal_move_physical_cursor_up(terminal_fd, rows_to_move_up);
    terminal_move_physical_cursor_right(terminal_fd, columns_to_move_right);
}

void terminal_cursor_reset(terminal_cursor_st * const terminal_cursor)
{
    terminal_cursor->row = 0;
    terminal_cursor->column = 0;
}

void terminal_cursor_init(terminal_cursor_st * const terminal_cursor)
{
    terminal_cursor->num_rows = 1;
    terminal_cursor_reset(terminal_cursor);
}
