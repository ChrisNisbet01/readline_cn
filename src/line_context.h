/* Copyright (C) Chris Nisbet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly 
 * prohibited. Proprietary and confidential. Written by Chris 
 * Nisbet <nisbet@ihug.co.nz>, April 2016.
 */

#ifndef __LINE_CONTEXT_H__
#define __LINE_CONTEXT_H__

#include <stdbool.h>
#include <stddef.h>

typedef struct terminal_cursor_st terminal_cursor_st;
struct terminal_cursor_st
{
    size_t row;
    size_t column;
    size_t num_rows; /* The number of rows on the terminal the line occupies. */
}; 

typedef struct line_context_st line_context_st;
/* This structure mostly contains variables that are only 
 * needed during a single call to readline(). 
 */
struct line_context_st
{
    size_t size_increment;
    char * edit_buffer; /* Storage for the line being edited. */
    size_t buffer_size; /* Amount of memory alloced for the line. */
    size_t line_length; /* Current length of the line. */
    size_t maximum_line_length;
    size_t edit_index; /* Location of the cursor in the line. */
    int terminal_fd; /* The file descriptor to write to when updating the terminal. */
    size_t terminal_width;
    int mask_character; /* if non-zero, the character to write to the terminal instead of the actual character entered. */
    char const * prompt;
    bool any_chars_read; /* initially false, then true once any characters have been read. */

    terminal_cursor_st terminal_cursor;
};

bool line_context_init(line_context_st * const line_context,
                       size_t const initial_size, 
                       size_t const size_increment,
                       size_t const maximum_line_length,
                       int const terminal_fd,
                       size_t const terminal_width,
                       int const mask_character,
                       char const * const prompt);
void line_context_teardown(line_context_st * const line_context);

void move_cursor_right_n_columns(line_context_st * const line_ctx, size_t columns);
void move_cursor_left_n_columns(line_context_st * const line_ctx, size_t const columns);

void delete_char_to_the_left(line_context_st * const line_ctx);
void delete_char_to_the_right(line_context_st * const line_ctx, bool const update_display);
void delete_chars_to_the_right(line_context_st * const line_ctx, size_t const chars_to_delete);
void delete_chars_to_the_left(line_context_st * const line_ctx, size_t const chars_to_delete);

void write_char(line_context_st * const line_ctx, int const ch, bool const insert_mode, bool const update_terminal);
void write_string(line_context_st * const line_ctx, char const * const string, bool insert_mode, bool const update_terminal);

void complete_word(line_context_st * const line_ctx, char const * const completion, bool const update_terminal);
void replace_edit_line(line_context_st * const line_ctx, char const * const replacement);
void redisplay_line(line_context_st * const line_ctx);
void free_saved_string(char const * * const saved_line);
void save_string(line_context_st * const line_ctx, char const * * const destination); 
void delete_from_cursor_to_end(line_context_st * const line_ctx);
void delete_from_start_to_cursor(line_context_st * const line_ctx);
void transpose_characters(line_context_st * const line_ctx);

size_t get_index_of_start_of_previous_word(line_context_st * const line_ctx);
size_t get_index_of_end_of_next_word(line_context_st * const line_ctx);

void move_left_to_beginning_of_word(line_context_st * const line_ctx);
void move_right_to_end_of_word(line_context_st * const line_ctx);

void delete_previous_word(line_context_st * const line_ctx);
void delete_to_next_word(line_context_st * const line_ctx);


#endif /* __LINE_CONTEXT_H__ */
