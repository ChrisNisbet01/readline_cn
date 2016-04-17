#include <CppUTest/TestHarness.h>

extern "C"
{
#include "tokenise.h"
};

TEST_GROUP(tokenise)
{
    tokens_st * tokens;

    void setup()
    {
        tokens = NULL;
    }

    void teardown()
    {
        /* cleanup */
        tokens_free(tokens);
    }

    void do_test(char const * const line, size_t const expected_num_tokens, char const * const * const expected_tokens)
    {
        size_t index;

        /* perform test */
        tokens = tokenise_line(line, 0, 0, false);

        /* check results */
        LONGS_EQUAL(expected_num_tokens, tokens_get_num_tokens(tokens));
        if (expected_tokens != NULL)
        {
            for (index = 0; index < expected_num_tokens; index++)
            {
                STRCMP_EQUAL(expected_tokens[index], tokens_get_token_at_index(tokens, index));
            }
        }
    }
};

TEST(tokenise, empty_line_returns_no_tokens)
{
    do_test("", 0, NULL);
}

TEST(tokenise, all_spaces_returns_no_tokens)
{
    do_test("   ", 0, NULL);
}

TEST(tokenise, one_word_gives_one_token)
{
    char const * expected_tokens[] = {"test"};

    do_test("test", 1, expected_tokens);
}

TEST(tokenise, one_word_leading_whitespace_gives_one_token)
{
    char const * expected_tokens[] = {"test"};

    do_test(" test", 1, expected_tokens);
}

TEST(tokenise, one_word_trailing_whitespace_gives_one_token)
{
    char const * expected_tokens[] = {"test"};

    do_test("test ", 1, expected_tokens);
}

TEST(tokenise, one_word_leading_and_trailing_whitespace_gives_one_token)
{
    char const * expected_tokens[] = {"test"};

    do_test(" test ", 1, expected_tokens); 
}

TEST(tokenise, two_words_gives_two_tokens)
{
    char const * expected_tokens[] = {"test1", "test2"};

    do_test("test1 test2", 2, expected_tokens);
}

TEST(tokenise, quoted_word_includes_quotes)
{
    char const * expected_tokens[] = {"\"test1\"", "\"test2\""};

    do_test("\"test1\" \"test2\"", 2, expected_tokens);
}

TEST(tokenise, quoted_word_with_embedded_space)
{
    char const * expected_tokens[] = {"\"test1 test2\""};

    do_test("\"test1 test2\"", 1, expected_tokens);
}

TEST(tokenise, quoted_word_with_no_closing_quote)
{
    char const * expected_tokens[] = {"\"test1 test2"};

    do_test("\"test1 test2", 1, expected_tokens);
}

TEST(tokenise, quoted_word_with_no_closing_quote_and_trailing_whitespace)
{
    char const * expected_tokens[] = {"\"test1 test2 "};

    do_test("\"test1 test2 ", 1, expected_tokens);
}

TEST(tokenise, word_with_embedded_quote_not_split_in_two)
{
    char const * expected_tokens[] = {"test1\"test2"};

    do_test("test1\"test2", 1, expected_tokens);
}

TEST_GROUP(tokenise_cursor_index)
{
    tokens_st * tokens;

    void setup()
    {
        tokens = NULL;
    }

    void teardown()
    {
        /* cleanup */
        tokens_free(tokens);
    }

    void do_test(char const * const line, 
                 size_t const cursor_index,
                 size_t const expected_current_token_index, 
                 char const * const expected_current_token)
    {
        /* perform test */
        tokens = tokenise_line(line, 0, cursor_index, true);

        /* check results */
        CHECK_TRUE(expected_current_token_index <= tokens_get_num_tokens(tokens));
        LONGS_EQUAL(expected_current_token_index, tokens_get_current_token_index(tokens));
        if (expected_current_token != NULL)
        {
            STRCMP_EQUAL(expected_current_token, tokens_get_current_token(tokens));
        }
    }
};

TEST(tokenise_cursor_index, empty_line_returns_empty_current_token)
{
    do_test("", 0, 0, "");
}

TEST(tokenise_cursor_index, cursor_within_first_word_returns_index_0)
{
    do_test("test", 1, 0, "t");
}

TEST(tokenise_cursor_index, cursor_beginning_of_first_word_returns_index_0)
{
    do_test("test", 0, 0, "");
}

TEST(tokenise_cursor_index, cursor_before_first_word_returns_index_0)
{
    do_test("   test", 1, 0, "");
}

TEST(tokenise_cursor_index, cursor_in_first_word_returns_index_0)
{
    do_test("   test", 4, 0, "t");
}

TEST(tokenise_cursor_index, beginning_of_second_token)
{
    do_test("abc def", 4, 1, "");
}

TEST(tokenise_cursor_index, within_second_token)
{
    do_test("abc def", 6, 1, "de");
}

TEST(tokenise_cursor_index, end_of_token)
{
    do_test("abc def", 7, 1, "def");
}

TEST(tokenise_cursor_index, after_last_token)
{
    do_test("abc def  ", 8, 2, "");
}

TEST(tokenise_cursor_index, beyond_line_length)
{
    do_test("abc def  ", 12, 2, NULL);
}

