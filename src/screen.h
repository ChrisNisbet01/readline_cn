#ifndef __SCREEN_H__
#define __SCREEN_H__

#include "line_context.h"

void screen_put(line_context_st * const line_ctx, char const ch);
void screen_puts(line_context_st * const line_ctx, char const * const string, char const mask_character);
void move_terminal_cursor_right_n_columns(line_context_st * const line_ctx, size_t const columns);
void move_terminal_cursor_left_n_columns(line_context_st * const line_ctx, size_t const columns);
void terminal_cursor_init(terminal_cursor_st * const terminal_cursor, int const terminal_fd, size_t const terminal_width);
void terminal_cursor_reset_cursor(terminal_cursor_st * const terminal_cursor);

#endif /* __SCREEN_H__ */
