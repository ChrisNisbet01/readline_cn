/* Copyright (C) Chris Nisbet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly 
 * prohibited. Proprietary and confidential. Written by Chris 
 * Nisbet <nisbet@ihug.co.nz>, April 2016.
 */

#include <CppUTest/TestHarness.h>

extern "C"
{
#include "common_prefix_length.h"
};

#include <string.h>
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

TEST_GROUP(common_prefix_length)
{
    void setup()
    {
    }

    void teardown()
    {
    }
};

TEST(common_prefix_length, single_word_returns_word_length)
{
    char const * const words[] = { "word" };
    size_t common_prefix_length;

    /* setup */

    /* perform test */
    common_prefix_length = find_common_prefix_length(ARRAY_SIZE(words), words);

    /* check results */
    LONGS_EQUAL(strlen(words[0]), common_prefix_length);
}

TEST(common_prefix_length, multiple_words_word0_shortest)
{
    char const * const words[] = { "word", "word1" };
    size_t common_prefix_length;

    /* setup */

    /* perform test */
    common_prefix_length = find_common_prefix_length(ARRAY_SIZE(words), words);

    /* check results */
    LONGS_EQUAL(strlen(words[0]), common_prefix_length);
}

TEST(common_prefix_length, multiple_words_word1_shortest)
{
    char const * const words[] = { "word0", "word" };
    size_t common_prefix_length;

    /* setup */

    /* perform test */
    common_prefix_length = find_common_prefix_length(ARRAY_SIZE(words), words);

    /* check results */
    LONGS_EQUAL(strlen(words[1]), common_prefix_length);
}

TEST(common_prefix_length, no_common_prefix)
{
    char const * const words[] = { "abc", "def" };
    size_t common_prefix_length;

    /* setup */

    /* perform test */
    common_prefix_length = find_common_prefix_length(ARRAY_SIZE(words), words);

    /* check results */
    LONGS_EQUAL(0, common_prefix_length);
}

TEST(common_prefix_length, first_word_empty_string_returns_0)
{
    char const * const words[] = { "", "def" };
    size_t common_prefix_length;

    /* setup */

    /* perform test */
    common_prefix_length = find_common_prefix_length(ARRAY_SIZE(words), words);

    /* check results */
    LONGS_EQUAL(0, common_prefix_length);
}

TEST(common_prefix_length, second_word_empty_string_returns_0)
{
    char const * const words[] = { "abc", "" };
    size_t common_prefix_length;

    /* setup */

    /* perform test */
    common_prefix_length = find_common_prefix_length(ARRAY_SIZE(words), words);

    /* check results */
    LONGS_EQUAL(0, common_prefix_length);
}

TEST(common_prefix_length, partial_common_prefix)
{
    char const * const words[] = { "wordabc", "worddef" };
    size_t common_prefix_length;

    /* setup */

    /* perform test */
    common_prefix_length = find_common_prefix_length(ARRAY_SIZE(words), words);

    /* check results */
    LONGS_EQUAL(strlen("word"), common_prefix_length);
}

TEST(common_prefix_length, nulls_ignored)
{
    char const * const words[] = { "wordabc", NULL, "worddef" };
    size_t common_prefix_length;

    /* setup */

    /* perform test */
    common_prefix_length = find_common_prefix_length(ARRAY_SIZE(words), words);

    /* check results */
    LONGS_EQUAL(strlen("word"), common_prefix_length);
}

