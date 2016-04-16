#include "common_prefix_length.h"

#include <stdbool.h>
#include <string.h>

static bool word_differs_at_index(size_t const index_to_check, size_t const num_words, char const * const * const words)
{
    bool found_difference;
    size_t word_index;

    found_difference = false;
    for (word_index = 1; !found_difference && word_index < num_words; word_index++)
    {
        if (words[0][index_to_check] != words[word_index][index_to_check])
        {
            found_difference = true;
        }
    }

    return found_difference;
}

size_t find_common_prefix_length(size_t const num_words, char const * const * const words)
{
    size_t char_index;
    size_t const length_to_compare = strlen(words[0]);

    /* Find largest matching substring. */
    for (char_index = 0; char_index < length_to_compare; char_index++)
    {
        if (word_differs_at_index(char_index, num_words, words))
        {
            break;
        }
    }

    return char_index;
}


