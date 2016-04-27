#ifndef __TERMINAL_CURSOR_H__
#define __TERMINAL_CURSOR_H__

#include "line_context.h"

void terminal_put(terminal_cursor_st * const terminal_cursor, char const ch);
void terminal_puts(terminal_cursor_st * const terminal_cursor, char const * const string, char const mask_character);
void terminal_move_cursor_right_n_columns(terminal_cursor_st * const terminal_cursor, size_t const columns);
void terminal_move_cursor_left_n_columns(terminal_cursor_st * const terminal_cursor, size_t const columns);
void terminal_delete_line_from_cursor_to_end(terminal_cursor_st * const terminal_cursor);
void terminal_cursor_init(terminal_cursor_st * const terminal_cursor, int const terminal_fd, size_t const terminal_width);
void terminal_cursor_reset(terminal_cursor_st * const terminal_cursor);

#endif /* __TERMINAL_CURSOR_H__ */
