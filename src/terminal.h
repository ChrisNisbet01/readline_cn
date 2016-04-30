/* Copyright (C) Chris Nisbet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly 
 * prohibited. Proprietary and confidential. Written by Chris 
 * Nisbet <nisbet@ihug.co.nz>, April 2016.
 */

#ifndef __TERMINAL_H__
#define __TERMINAL_H__

#include <termios.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum tty_get_result_t
{
    tty_get_result_ok,
    tty_get_result_eof,
    tty_get_result_timeout
} tty_get_result_t;

void tty_put(int const out_fd, char const c);
void tty_puts(int const out_fd, char const * const string);
tty_get_result_t tty_get(int const in_fd, unsigned int const maximum_seconds_to_wait, int * const character_read);

void terminal_prepare(struct termios * const previous_terminal_settings);
void terminal_restore(struct termios * const previous_terminal_settings);
size_t terminal_get_width(int const out_fd);
bool terminal_move_physical_cursor_right(int const out_fd, size_t const columns);
bool terminal_move_physical_cursor_left(int const out_fd, size_t const columns);
bool terminal_move_physical_cursor_up(int const out_fd, size_t const rows);
bool terminal_move_physical_cursor_down(int const out_fd, size_t const rows);

void terminal_delete_to_end_of_line(int const out_fd);

#endif /* __TERMINAL_H__ */
