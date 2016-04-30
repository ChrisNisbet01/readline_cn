/* Copyright (C) Chris Nisbet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly 
 * prohibited. Proprietary and confidential. Written by Chris 
 * Nisbet <nisbet@ihug.co.nz>, April 2016.
 */

#include "common_prefix_length.h"

#include <stdbool.h>
#include <string.h>

static bool word_differs_at_index(size_t index, char const * const word1, char const * const word2)
{
    return word1 != NULL && word2 != NULL && word1[index] != word2[index];
}

static char const * find_first_non_null_word(size_t const num_words, char const * const * const words)
{
    char const * word;
    size_t index;

    for (index = 0; index < num_words; index++)
    {
        if (words[index] != NULL)
        {
            word = words[index];
            goto done;
        }
    }
    word = NULL;

done:
    return word;
}

static bool any_word_differs_at_index(size_t const index_to_check, size_t const num_words, char const * const * const words)
{
    bool found_difference;
    size_t word_index;
    char const * comparison_word;

    found_difference = false;

    comparison_word = find_first_non_null_word(num_words, words);
    if (comparison_word == NULL)
    {
        goto done;
    }

    for (word_index = 0; !found_difference && word_index < num_words; word_index++)
    {
        if (word_differs_at_index(index_to_check, comparison_word, words[word_index]))
        {
            found_difference = true;
        }
    }

done:
    return found_difference;
}

size_t find_common_prefix_length(size_t const num_words, char const * const * const words)
{
    size_t char_index;
    size_t const length_to_compare = strlen(words[0]);

    /* Find largest matching substring. */
    for (char_index = 0; char_index < length_to_compare; char_index++)
    {
        if (any_word_differs_at_index(char_index, num_words, words))
        {
            break;
        }
    }

    return char_index;
}


