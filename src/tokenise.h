#ifndef __TOKENISE_H__
#define __TOKENISE_H__

#include <stddef.h>
#include <stdbool.h>

typedef struct tokens_st tokens_st;

size_t tokens_get_current_token_index(tokens_st const * const tokens);
char const * tokens_get_current_token(tokens_st const * const tokens);
size_t tokens_get_num_tokens(tokens_st const * const tokens);
char const * tokens_get_token_at_index(tokens_st const * const tokens, size_t const index);

void tokens_free(tokens_st * const tokens);

/* Find the start and end indexes for all words on the current 
 * line. In addition, if the cursor is between lines, add in 
 * an entry for this as well. Use an empty string to represent 
 * the word in that case. 
 * Return the number of line_indexes found. 
 */
tokens_st * tokenise_line(char const * const line,
                          size_t const start_index,
                          size_t const cursor_index,
                          bool const assign_token_to_cursor_index,
                          char const * const field_separators);

#endif /* __TOKENISE_H__ */
