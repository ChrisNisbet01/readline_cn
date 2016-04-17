#ifndef __LINE_CONTEXT_H__
#define __LINE_CONTEXT_H__

#include <stdbool.h>
#include <stddef.h>

typedef struct line_context_st line_context_st;
struct line_context_st
{
    size_t size_increment;
    char * buffer; /* Storage for the line being edited. */
    size_t buffer_size; /* Amount of memory alloced for the line. */
    size_t line_length; /* Current length of the line. */
    size_t cursor_index; /* Location of the cursor in the line. */
    int terminal_fd; /* The file descriptor to write to when updating the terminal. */
    int mask_character; /* if non-zero, the character to write to the terminal instead of the actual character entered. */
}; 

bool line_context_init(line_context_st * const line_context,
                      size_t const initial_size, 
                      size_t const size_increment,
                      int const terminal_fd,
                      int const mask_character);
void line_context_teardown(line_context_st * const line_context);
void move_cursor_right_n_columns(line_context_st * const line_ctx, size_t columns);
void move_cursor_left_n_columns(line_context_st * const line_ctx, size_t const columns);
void delete_char_to_the_left(line_context_st * const line_ctx);
void delete_char_to_the_right(line_context_st * const line_ctx, bool const update_display);
void delete_char(line_context_st * const line_ctx, bool update_display);
void write_char(line_context_st * const line_ctx, int const ch, bool const insert_mode, bool const update_terminal);
void write_string(line_context_st * const line_ctx, char const * const string, bool insert_mode, bool const update_terminal);
void complete_word(line_context_st * const line_ctx, char const * const completion, bool const update_terminal);
void replace_line(line_context_st * const line_ctx, char const * const replacement);


#endif /* __LINE_CONTEXT_H__ */
