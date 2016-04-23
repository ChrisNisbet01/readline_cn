#include "tokenise.h"
#include "strdup_partial.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

typedef struct token_st token_st;
struct token_st
{
    size_t start_index;
    size_t end_index;
    char * word;
};

struct tokens_st
{
    size_t current_token_index;
    char const * current_token;
    size_t count;
    size_t token_array_size;
    token_st * token_array;
}; 

static bool tokens_ensure_space_for_new_token(tokens_st * const tokens)
{
    bool has_space;
    token_st * new_token_array;
    size_t new_token_array_size;

    if (tokens->count < tokens->token_array_size)
    {
        has_space = true;
        goto done;
    }
    /* Make space for another token. */
    new_token_array_size = tokens->token_array_size + 1;
    new_token_array = calloc(new_token_array_size, sizeof *new_token_array);
    if (new_token_array == NULL)
    {
        has_space = false;
        goto done;
    }
    memcpy(new_token_array, tokens->token_array, tokens->token_array_size * sizeof *new_token_array);
    free(tokens->token_array);
    tokens->token_array = new_token_array;
    tokens->token_array_size = new_token_array_size;
    has_space = true;

done:
    return has_space;
}

static tokens_st * tokens_alloc(void)
{
    tokens_st * tokens;

    tokens = calloc(1, sizeof *tokens);
    if (tokens == NULL)
    {
        goto done;
    }

done:
    return tokens;
}

static bool check_if_cursor_index_within_word(size_t const cursor_index, size_t const start_index, size_t const end_index)
{
    bool cursor_is_within_word;

    if (cursor_index >= start_index && cursor_index <= end_index)
    {
        cursor_is_within_word = true;
    }
    else
    {
        cursor_is_within_word = false;
    }

    return cursor_is_within_word;
}

static bool create_cursor_token_if_cursor_index_fits_in_token(char const * const line,
                                                              size_t const cursor_index,
                                                              tokens_st * const tokens)
{
    bool cursor_falls_within_token;

    cursor_falls_within_token = check_if_cursor_index_within_word(cursor_index,
                                                                  tokens->token_array[tokens->count].start_index,
                                                                  tokens->token_array[tokens->count].end_index);

    if (cursor_falls_within_token)
    {
        tokens->current_token_index = tokens->count;
        /* Only include the part of the word from the start to the 
         * current cursor position. 
         */
        tokens->current_token = strdup_partial(line,
                                               tokens->token_array[tokens->count].start_index,
                                               cursor_index);
    }

    return cursor_falls_within_token;
}

static bool populate_next_token(tokens_st * const tokens,
                                char const * const line, 
                                size_t const start_index, 
                                size_t const end_index,
                                bool const assign_token_to_cursor_index,
                                bool * const done_cursor_index_token,
                                size_t cursor_index)
{
    bool populated_ok;
    token_st * token;

    if (!tokens_ensure_space_for_new_token(tokens))
    {
        populated_ok = false;
        goto done;
    }
    token = &tokens->token_array[tokens->count];
    token->word = strdup_partial(line, start_index, end_index);

    if (token->word == NULL)
    {
        populated_ok = false;
        goto done;
    }

    token->start_index = start_index;
    token->end_index = end_index;

    if (assign_token_to_cursor_index)
    {
        if (!(*done_cursor_index_token))
        {
            *done_cursor_index_token =
                create_cursor_token_if_cursor_index_fits_in_token(line,
                                                                  cursor_index,
                                                                  tokens);
        }
    }

    tokens->count++; 


    populated_ok = true;

done:
    return populated_ok;
}

size_t tokens_get_current_token_index(tokens_st const * const tokens)
{
    size_t current_token_index;

    if (tokens != NULL)
    {
        current_token_index = tokens->current_token_index; 
    }
    else
    {
        current_token_index = 0;
    }

    return current_token_index;
}

char const * tokens_get_current_token(tokens_st const * const tokens)
{
    char const * current_token;

    if (tokens != NULL)
    {
        current_token = tokens->current_token;
    }
    else
    {
        current_token = NULL;
    }

    return current_token;
}

size_t tokens_get_num_tokens(tokens_st const * const tokens)
{
    size_t count;

    if (tokens != NULL)
    {
        count = tokens->count;
    }
    else
    {
        count = 0;
    }

    return count;
}

char const * tokens_get_token_at_index(tokens_st const * const tokens, size_t const index)
{
    char const * token;

    if (tokens == NULL)
    {
        token = NULL;
        goto done;
    }
    if (index >= tokens->count)
    {
        token = NULL;
        goto done;
    }
    token = tokens->token_array[index].word;

done:
    return token;
}

void tokens_free(tokens_st * const tokens)
{
    if (tokens != NULL)
    {
        if (tokens->token_array)
        {
            size_t index;

            for (index = 0; index < tokens->count; index++)
            {
                free(tokens->token_array[index].word);
            }
            free(tokens->token_array);
        }
        free(tokens);
    }
}

static bool char_is_field_separator(char const * const field_separators, char const ch)
{
    bool const is_a_field_separator = field_separators != NULL && strchr(field_separators, (int)ch) != NULL; 

    return is_a_field_separator;
}

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
                          char const * const field_separators)
{
    tokens_st * tokens;
    size_t current_index;
    size_t token_start_index;
    bool done_cursor_index_token;
    char const double_quote_delimiter = '\"';
    char const newline = '\n';
    char const nul = '\0';
    enum token_type_t
    {
        token_type_none,
        token_type_plain,
        token_type_double_quoted
    };
    enum token_type_t token_type;

    tokens = tokens_alloc();
    if (tokens == NULL)
    {
        goto done;
    }

    done_cursor_index_token = false;
    token_type = token_type_none;
    current_index = start_index;
    token_start_index = current_index; /* Avoid compiler warning. */

    for (current_index = start_index;
         line[current_index] != nul && line[current_index] != newline;
         current_index++)
    {
        char ch = line[current_index];

        if (token_type == token_type_double_quoted)
        {
            if (ch == double_quote_delimiter)
            {
                size_t const index_of_end_of_word = current_index + 1; /* include the terminating double quote */

                populate_next_token(tokens, 
                                    line, 
                                    token_start_index, 
                                    index_of_end_of_word, 
                                    assign_token_to_cursor_index,
                                    &done_cursor_index_token,
                                    cursor_index);
                token_type = token_type_none;
            }
        }
        else if (isspace((int)ch))
        {
            if (token_type == token_type_plain)
            {
                populate_next_token(tokens, 
                                    line, 
                                    token_start_index, 
                                    current_index,
                                    assign_token_to_cursor_index,
                                    &done_cursor_index_token,
                                    cursor_index);
                token_type = token_type_none;
            }
            else if (current_index == cursor_index)
            {
                if (assign_token_to_cursor_index)
                {
                    if (!done_cursor_index_token)
                    {
                        populate_next_token(tokens, 
                                            NULL, 
                                            cursor_index, 
                                            cursor_index,
                                            assign_token_to_cursor_index,
                                            &done_cursor_index_token,
                                            cursor_index);
                    }
                }
            }
        }
        else if (char_is_field_separator(field_separators, ch))
        {
            if (token_type == token_type_plain)
            {
                populate_next_token(tokens, 
                                    line, 
                                    token_start_index, 
                                    current_index,
                                    assign_token_to_cursor_index,
                                    & done_cursor_index_token,
                                    cursor_index);
                token_type = token_type_none;
            }
            else if (current_index == cursor_index)
            {
                if (assign_token_to_cursor_index)
                {
                    if (!done_cursor_index_token)
                    {
                        populate_next_token(tokens, 
                                            NULL, 
                                            cursor_index, 
                                            cursor_index,
                                            assign_token_to_cursor_index,
                                            &done_cursor_index_token,
                                            cursor_index);
                    }
                }
            }
            if (token_type == token_type_none)
            {
                /* Create a token that includes just the pipe character. */
                populate_next_token(tokens, 
                                    line, 
                                    current_index, 
                                    current_index + 1,
                                    assign_token_to_cursor_index,
                                    & done_cursor_index_token,
                                    cursor_index);
            }
        }
        else if (ch == double_quote_delimiter)
        {
            if (token_type == token_type_none) /* ignore double quotes embedded in words */
            {
                token_start_index = current_index; /* include the leading double quote */
                token_type = token_type_double_quoted;
            }
        }
        else
        {
            if (token_type == token_type_none)
            {
                token_start_index = current_index;
                token_type = token_type_plain;
            }
        }
    }

    if (token_type != token_type_none) /* Line has chars at the end of the line. */
    {
        populate_next_token(tokens, 
                            line, 
                            token_start_index, 
                            current_index,
                            assign_token_to_cursor_index,
                            & done_cursor_index_token,
                            cursor_index);
    }

    if (assign_token_to_cursor_index)
    {
        if (!done_cursor_index_token)
        {
            /* The line must have whitespace chars before the end of the 
             * line, and the cursor must also be at the end of the line. 
             */

            populate_next_token(tokens, 
                                NULL, 
                                cursor_index, 
                                cursor_index,
                                assign_token_to_cursor_index,
                                &done_cursor_index_token,
                                cursor_index);
        }
    }

done:

    return tokens;
}


