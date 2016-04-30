/* Copyright (C) Chris Nisbet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly 
 * prohibited. Proprietary and confidential. Written by Chris 
 * Nisbet <nisbet@ihug.co.nz>, April 2016.
 */

#include <CppUTest/TestHarness.h>

extern "C"
{
#include "history.h"
};

TEST_GROUP(history)
{
    history_st * history;

    void setup()
    {
    }

    void teardown()
    {
        history_free(history);
        history = NULL;
    }
};

TEST(history, alloc_unlimited_returns_non_null)
{
    size_t maximum_size;

    /* setup */
    maximum_size = 0;

    /* perform_test */
    history = history_alloc(maximum_size);

    /* check_results */
    CHECK_TRUE(history != NULL);
}

TEST(history, add_one_entry_when_max_size_specified)
{
    size_t maximum_size;
    bool entry_added;
    /* setup */
    maximum_size = 1;
    history = history_alloc(maximum_size); 

    /* perform_test */
    entry_added = history_add(history, "test_string");

    /* check_results */
    CHECK_TRUE(entry_added);
}

TEST(history, add_one_entry_when_max_size_unlimited)
{
    size_t maximum_size;
    bool entry_added;
    /* setup */
    maximum_size = 0;
    history = history_alloc(maximum_size);

    /* perform_test */
    entry_added = history_add(history, "test_string");

    /* check_results */
    CHECK_TRUE(entry_added);
}

TEST(history, add_whitespace_fails)
{
    size_t maximum_size;
    bool entry_added;
    char const whitespace_string[] = " ";

    /* setup */
    maximum_size = 1;
    history = history_alloc(maximum_size);

    /* perform_test */
    entry_added = history_add(history, whitespace_string);

    /* check_results */
    CHECK_FALSE(entry_added);
}

TEST(history, add_duplicate_of_newest_fails)
{
    size_t maximum_size;
    bool first_entry_added;
    bool duplicate_entry_added;
    char const test_string[] = "test_string";

    /* setup */
    maximum_size = 1;
    history = history_alloc(maximum_size);

    /* perform_test */
    first_entry_added = history_add(history, test_string);
    duplicate_entry_added = history_add(history, test_string);

    /* check_results */
    CHECK_TRUE(first_entry_added);
    CHECK_FALSE(duplicate_entry_added);
}

TEST(history, most_recent_entry_matches_last_added)
{
    size_t maximum_size;
    char const * most_recent;
    char const test_string[] = "test_string";

    /* setup */
    maximum_size = 1;
    history = history_alloc(maximum_size);

    /* perform_test */
    (void)history_add(history, test_string);
    most_recent = history_get_older_entry(history);

    /* check_results */
    STRCMP_EQUAL(test_string, most_recent);
}

TEST(history, begins_at_most_recent)
{
    size_t maximum_size;
    bool at_most_recent;

    /* setup */
    maximum_size = 1;
    history = history_alloc(maximum_size);

    /* perform_test */
    at_most_recent = history_currently_at_most_recent(history);

    /* check_results */
    CHECK_TRUE(at_most_recent);
}

TEST(history, not_at_most_recent_after_getting_older_entry)
{
    size_t maximum_size;
    bool at_most_recent;
    char const test_string[] = "test_string"; 

    /* setup */
    maximum_size = 1;
    history = history_alloc(maximum_size);

    /* perform_test */
    /* add a string */
    (void)history_add(history, test_string); 
    /* now get an older entry */
    (void)history_get_older_entry(history);

    at_most_recent = history_currently_at_most_recent(history);

    /* check_results */
    CHECK_FALSE(at_most_recent);
}

TEST(history, at_most_recent_after_adding_entry)
{
    size_t maximum_size;
    bool at_most_recent;
    char const test_string[] = "test_string";

    /* setup */
    maximum_size = 1;
    history = history_alloc(maximum_size);

    /* perform_test */
    /* add a string */
    (void)history_add(history, test_string);

    at_most_recent = history_currently_at_most_recent(history);

    /* check_results */
    CHECK_TRUE(at_most_recent);
}

TEST(history, entries_returned_in_correct_order)
{
    size_t maximum_size;
    char const test_string1[] = "test_string1";
    char const test_string2[] = "test_string2"; 
    char const * first_entry_retrieved;
    char const * second_entry_retrieved;

    /* setup */
    maximum_size = 2;
    history = history_alloc(maximum_size);

    /* perform_test */
    (void)history_add(history, test_string1);
    (void)history_add(history, test_string2);
    /* Retrieve the entries. */
    first_entry_retrieved = history_get_older_entry(history);
    second_entry_retrieved = history_get_older_entry(history);

    /* check_results */
    STRCMP_EQUAL(test_string2, first_entry_retrieved);
    STRCMP_EQUAL(test_string1, second_entry_retrieved);
}

TEST(history, history_size_honoured)
{
    size_t maximum_size;
    char const test_string1[] = "test_string1";
    char const test_string2[] = "test_string2";
    char const * first_entry_retrieved;
    char const * second_entry_retrieved;

    /* setup */
    maximum_size = 1;
    history = history_alloc(maximum_size);

    /* perform_test */
    (void)history_add(history, test_string1);
    (void)history_add(history, test_string2);
    /* Retrieve the entries. */
    first_entry_retrieved = history_get_older_entry(history);
    second_entry_retrieved = history_get_older_entry(history);

    /* check_results */
    STRCMP_EQUAL(test_string2, first_entry_retrieved);
    STRCMP_EQUAL(NULL, second_entry_retrieved);
}

TEST(history, newer_entries_retrieved_successfully)
{
    size_t maximum_size;
    char const test_string1[] = "test_string1";
    char const test_string2[] = "test_string2";
    char const * first_entry_retrieved;
    char const * second_entry_retrieved;
    char const * newer_entry;
    char const * second_newer_entry;

    /* setup */
    maximum_size = 2;
    history = history_alloc(maximum_size);

    /* perform_test */
    (void)history_add(history, test_string1);
    (void)history_add(history, test_string2);
    /* Retrieve the entries. */
    first_entry_retrieved = history_get_older_entry(history);
    second_entry_retrieved = history_get_older_entry(history);
    newer_entry = history_get_newer_entry(history);
    second_newer_entry = history_get_newer_entry(history);

    /* check_results */
    STRCMP_EQUAL(test_string2, first_entry_retrieved);
    STRCMP_EQUAL(test_string1, second_entry_retrieved);
    STRCMP_EQUAL(test_string2, newer_entry);
    STRCMP_EQUAL(NULL, second_newer_entry);
}

TEST(history, at_most_recent_after_getting_older_then_newer)
{
    size_t maximum_size;
    char const test_string1[] = "test_string1";
    bool at_most_recent;

    /* setup */
    maximum_size = 2;
    history = history_alloc(maximum_size);

    /* perform_test */
    (void)history_add(history, test_string1);
    /* Retrieve the entries. */
    (void)history_get_older_entry(history);
    (void)history_get_newer_entry(history);
    at_most_recent = history_currently_at_most_recent(history);

    /* check_results */
    CHECK_TRUE(at_most_recent);
}

TEST(history, reset_returns_to_most_recent)
{
    size_t maximum_size;
    char const test_string1[] = "test_string1";
    bool at_most_recent;

    /* setup */
    maximum_size = 2;
    history = history_alloc(maximum_size);

    /* perform_test */
    (void)history_add(history, test_string1);
    /* Retrieve the entries. */
    (void)history_get_older_entry(history);
    history_reset(history);
    at_most_recent = history_currently_at_most_recent(history);

    /* check_results */
    CHECK_TRUE(at_most_recent);
}

