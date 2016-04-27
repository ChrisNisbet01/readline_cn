#ifndef __SCREEN_H__
#define __SCREEN_H__

#include "line_context.h"

typedef struct terminal_st terminal_st;
struct terminal_st
{
    int out_fd; /* The file descriptor to write to when updating the terminal. */
    size_t width;
    size_t cursor_row;
    size_t cursor_column;
    size_t num_rows; /* The number of rows on the terminal the line occupies. */
}; 


void screen_put(line_context_st * const line_ctx, char const ch);
void screen_puts(line_context_st * const line_ctx, char const * const string, char const mask_character);
void move_terminal_cursor_right_n_columns(line_context_st * const line_ctx, size_t const columns);
void move_terminal_cursor_left_n_columns(line_context_st * const line_ctx, size_t const columns);
void terminal_init(terminal_st * const terminal, int const out_fd, size_t const terminal_width);

#endif /* __SCREEN_H__ */
