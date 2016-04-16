#include "tokenise.h"
#include "strdup_partial.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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
    token_st * token_array;
}; 

static tokens_st * tokens_alloc(size_t const num_tokens)
{
    tokens_st * tokens;

    tokens = calloc(1, sizeof *tokens);
    if (tokens == NULL)
    {
        goto done;
    }

    tokens->token_array = calloc(num_tokens, sizeof *tokens->token_array);
    if (tokens->token_array == NULL)
    {
        free(tokens);
        tokens = NULL;
        goto done;
    }

done:
    return tokens;
}

static bool populate_token(token_st * const token, char const * const line, size_t const start_index, size_t const end_index)
{
    bool populated_ok;

    token->word = strdup_partial(line, start_index, end_index);
    if (token->word == NULL)
    {
        populated_ok = false;
        goto done;
    }

    token->start_index = start_index;
    token->end_index = end_index;

    populated_ok = true;

done:
    return populated_ok;
}

static bool check_cursor_index(size_t const cursor_index, size_t const start_index, size_t const end_index)
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

    if (check_cursor_index(cursor_index,
                           tokens->token_array[tokens->count].start_index,
                           tokens->token_array[tokens->count].end_index))
    {
        tokens->current_token_index = tokens->count;
        tokens->current_token = strdup_partial(line,
                                               tokens->token_array[tokens->count].start_index,
                                               cursor_index);
        cursor_falls_within_token = true;
    }
    else
    {
        cursor_falls_within_token = false;
    }

    return cursor_falls_within_token;
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
        size_t index;

        if (tokens->token_array)
        {
            for (index = 0; index < tokens->count; index++)
            {
                free(tokens->token_array[index].word);
            }
            free(tokens->token_array);
        }
        free(tokens);
    }
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
                                 bool const assign_token_to_cursor_index)
{
    char const * const line_start = &line[start_index];
    tokens_st * tokens;
    /* Add in space for one more to deal with the case where the 
     * cursor is between words. In this case we want to add in an 
     * empty arg and push subsequent words along one. 
     */
    size_t maximum_number_of_args = 1 + (strlen(line_start) / 2);
    size_t maximum_possible_tokens = maximum_number_of_args + 1;
    size_t current_index;
    size_t token_start_index;
    bool done_cursor_index_token;
    char const quoted_token_delimiter = '\"';
    char const newline = '\n';
    char const nul = '\0';
    enum token_type_t
    {
        token_type_none,
        token_type_plain,
        token_type_quoted
    };
    enum token_type_t token_type;

    tokens = tokens_alloc(maximum_possible_tokens);
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

        if (token_type == token_type_quoted)
        {
            if (ch == quoted_token_delimiter)
            {
                size_t index_of_end_of_word = current_index + 1; /* include the terminating double quote */
                populate_token(&tokens->token_array[tokens->count], line, token_start_index, index_of_end_of_word);

                if (assign_token_to_cursor_index)
                {
                    if (!done_cursor_index_token)
                    {
                        done_cursor_index_token =
                            create_cursor_token_if_cursor_index_fits_in_token(line,
                                                                              cursor_index,
                                                                              tokens);
                    }
                }

                tokens->count++;
                token_type = token_type_none;
            }
        }
        else if (isspace((int)ch))
        {
            if (token_type == token_type_plain)
            {
                populate_token(&tokens->token_array[tokens->count], line, token_start_index, current_index);

                if (assign_token_to_cursor_index)
                {
                    if (!done_cursor_index_token)
                    {
                        done_cursor_index_token =
                            create_cursor_token_if_cursor_index_fits_in_token(line,
                                                                              cursor_index,
                                                                              tokens);
                    }
                }

                tokens->count++;
                token_type = token_type_none;
            }
            else if (current_index == cursor_index)
            {
                if (assign_token_to_cursor_index)
                {
                    if (!done_cursor_index_token)
                    {
                        populate_token(&tokens->token_array[tokens->count], NULL, cursor_index, cursor_index);

                        done_cursor_index_token =
                            create_cursor_token_if_cursor_index_fits_in_token(line,
                                                                              cursor_index,
                                                                              tokens);

                        tokens->count++;
                    }
                }
            }
        }
        else if (ch == quoted_token_delimiter)
        {
            if (token_type == token_type_none) /* ignore double quotes embedded in words */
            {
                token_start_index = current_index; /* include the leading double quote */
                token_type = token_type_quoted;
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
        populate_token(&tokens->token_array[tokens->count], line, token_start_index, current_index);

        if (assign_token_to_cursor_index)
        {
            if (!done_cursor_index_token)
            {
                done_cursor_index_token =
                    create_cursor_token_if_cursor_index_fits_in_token(line,
                                                                      cursor_index,
                                                                      tokens);
            }
        }

        tokens->count++;
    }

    if (assign_token_to_cursor_index)
    {
        if (!done_cursor_index_token)
        {
            /* The line must have whitespace chars before the end of the 
             * line, and the cursor must also be at the end of the line. 
             */

            populate_token(&tokens->token_array[tokens->count], NULL, cursor_index, cursor_index);

            done_cursor_index_token =
                create_cursor_token_if_cursor_index_fits_in_token(line,
                                                                  cursor_index,
                                                                  tokens);

            tokens->count++;
        }
    }

done:

    return tokens;
}


