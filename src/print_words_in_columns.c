#include "print_words_in_columns.h"
#include "terminal.h"

#include <stddef.h>
#include <string.h>

static size_t get_longest_word_length(unsigned int const word_count, char const * * const words)
{
    size_t longest_word_length;
    unsigned int i;

    for (longest_word_length = 0, i = 0; i < word_count; i++)
    {
        size_t const word_length = strlen(words[i]);

        if (word_length > longest_word_length)
        {
            longest_word_length = word_length;
        }
    }

    return longest_word_length;
}

static void pad_column(int const out_fd, unsigned int const width_printed, unsigned int const column_width)
{
    unsigned int printed = width_printed;

    while (printed++ < column_width)
    {
        tty_put(out_fd, ' ');
    }
}

static void print_row(int const out_fd,
                      unsigned int const row,
                      unsigned int const rows,
                      unsigned int const word_count,
                      char const * * const words,
                      unsigned int const column_width)
{
    unsigned int word_index;

    for (word_index = row; word_index < word_count; word_index += rows)
    {
        size_t char_index;
        size_t word_length;
        char const * current_word;

        current_word = words[word_index];
        word_length = strlen(current_word);
        for (char_index = 0; char_index < word_length; char_index++)
        {
            tty_put(out_fd, current_word[char_index]);
        }
        if (word_index + rows < word_count)
        {
            pad_column(out_fd, word_length, column_width);
        }
    }
}

void print_words_in_columns(int const out_fd, int const terminal_width, unsigned int const word_count, char const * * const words)
{
    unsigned int row;
    unsigned int rows;
    size_t longest_word_length;
    unsigned int column_width;
    unsigned int words_per_row;
    unsigned int const minimum_gap_between_columns = 2;

    longest_word_length = get_longest_word_length(word_count, words);
    column_width = longest_word_length + minimum_gap_between_columns;
    words_per_row = terminal_width / column_width;
    rows = 1 + (word_count / words_per_row);

    tty_put(out_fd, '\n');
    for (row = 0; row < rows; row++)
    {
        print_row(out_fd, row, rows, word_count, words, column_width);
        if (row < rows - 1)
        {
            tty_put(out_fd, '\n');
        }
    }
}


