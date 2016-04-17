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


