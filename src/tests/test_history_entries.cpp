#include <CppUTest/TestHarness.h>

extern "C"
{
#include "history_entries.h"
};

TEST_GROUP(entries)
{
    history_entries_st * entries;

    void setup()
    {
        entries = history_entries_alloc();
    }

    void teardown()
    {
        history_entries_free(entries);
    }
};

TEST(entries, add_one_entry_gives_count_of_one)
{
    history_entry_st * entry;
    size_t expected_count;

    /* setup */
    expected_count = 1;

    /* perform test */
    entry = history_entry_alloc("");
    history_add_new_entry_to_list(entries, entry);

    /* check results */
    LONGS_EQUAL(expected_count, history_entries_get_count(entries));
}

TEST(entries, initial_count_is_zero)
{
    size_t expected_count;

    /* setup */
    expected_count = 0;

    /* perform test */

    /* check results */
    LONGS_EQUAL(expected_count, history_entries_get_count(entries));
}

TEST(entries, get_newest_single_entry_value)
{
    history_entry_st * entry;
    history_entry_st * newest_entry;
    char const * expected_string;
    char const test_string[] = "test";

    /* setup */
    expected_string = test_string;

    /* perform test */
    entry = history_entry_alloc(test_string);
    history_add_new_entry_to_list(entries, entry);
    newest_entry = history_get_newest_entry_from_list(entries);

    /* check results */
    POINTERS_EQUAL(entry, newest_entry);
    STRCMP_EQUAL(expected_string, history_entry_get_value(newest_entry));
}

TEST(entries, get_oldest_single_entry_value)
{
    history_entry_st * entry;
    history_entry_st * oldest_entry;
    char const * expected_string;
    char const test_string[] = "test";

    /* setup */
    expected_string = test_string;

    /* perform test */
    entry = history_entry_alloc(test_string);
    history_add_new_entry_to_list(entries, entry);
    oldest_entry = history_get_oldest_entry_from_list(entries);

    /* check results */
    POINTERS_EQUAL(entry, oldest_entry);
    STRCMP_EQUAL(expected_string, history_entry_get_value(oldest_entry));
}

TEST(entries, get_older_single_entry_when_null_entry_given)
{
    history_entry_st * entry;
    history_entry_st * expected_entry; 
    char const test_string[] = "test";

    /* setup */
    expected_entry = NULL;

    /* perform test */
    entry = history_entry_alloc(test_string);
    history_add_new_entry_to_list(entries, entry);
    entry = history_get_older_entry_from_list(entries, NULL);

    /* check results */
    POINTERS_EQUAL(expected_entry, entry);
}

TEST(entries, add_two_entries_gives_count_of_two)
{
    history_entry_st * entry;
    size_t expected_count;
    char const test_string1[] = "test1";
    char const test_string2[] = "test2";

    /* setup */
    expected_count = 2;

    /* perform test */
    entry = history_entry_alloc(test_string1);
    history_add_new_entry_to_list(entries, entry);
    entry = history_entry_alloc(test_string2);
    history_add_new_entry_to_list(entries, entry); 

    /* check results */
    LONGS_EQUAL(expected_count, history_entries_get_count(entries));
}

TEST(entries, two_entries_returned_in_expected_order)
{
    history_entry_st * entry;
    history_entry_st * oldest_entry;
    history_entry_st * newest_entry;
    char const test_string1[] = "test1";
    char const test_string2[] = "test2";
    history_entry_st * expected_oldest;
    history_entry_st * expected_newest;

    /* setup */

    /* perform test */
    entry = history_entry_alloc(test_string1);
    history_add_new_entry_to_list(entries, entry);
    expected_oldest = entry;

    entry = history_entry_alloc(test_string2);
    history_add_new_entry_to_list(entries, entry);
    expected_newest = entry;

    newest_entry = history_get_newest_entry_from_list(entries);
    oldest_entry = history_get_oldest_entry_from_list(entries);

    /* check results */
    POINTERS_EQUAL(expected_newest, newest_entry);
    POINTERS_EQUAL(expected_oldest, oldest_entry);
}

TEST(entries, two_entries_returned_in_expected_order_using_get_older)
{
    history_entry_st * entry;
    history_entry_st * older_entry;
    history_entry_st * newest_entry;
    char const test_string1[] = "test1";
    char const test_string2[] = "test2";
    history_entry_st * expected_older;
    history_entry_st * expected_newest;

    /* setup */

    /* perform test */
    entry = history_entry_alloc(test_string1);
    history_add_new_entry_to_list(entries, entry);
    expected_older = entry;

    entry = history_entry_alloc(test_string2);
    history_add_new_entry_to_list(entries, entry);
    expected_newest = entry;

    newest_entry = history_get_newest_entry_from_list(entries);
    older_entry = history_get_older_entry_from_list(entries, newest_entry);

    /* check results */
    POINTERS_EQUAL(expected_newest, newest_entry);
    POINTERS_EQUAL(expected_older, older_entry);
}

TEST(entries, two_entries_returned_in_expected_order_using_get_newer)
{
    history_entry_st * entry;
    history_entry_st * oldest_entry;
    history_entry_st * newer_entry;
    char const test_string1[] = "test1";
    char const test_string2[] = "test2";
    history_entry_st * expected_oldest;
    history_entry_st * expected_newer;

    /* setup */

    /* perform test */
    entry = history_entry_alloc(test_string1);
    history_add_new_entry_to_list(entries, entry);
    expected_oldest = entry;

    entry = history_entry_alloc(test_string2);
    history_add_new_entry_to_list(entries, entry);
    expected_newer = entry;

    oldest_entry = history_get_oldest_entry_from_list(entries);
    newer_entry = history_get_newer_entry_from_list(entries, oldest_entry);

    /* check results */
    POINTERS_EQUAL(expected_oldest, oldest_entry);
    POINTERS_EQUAL(expected_newer, newer_entry);
}

TEST(entries, attempt_to_get_older_than_oldest_returns_null)
{
    history_entry_st * entry;
    history_entry_st * older_entry;
    char const test_string1[] = "test1";
    history_entry_st * expected_older;

    /* setup */

    /* perform test */
    entry = history_entry_alloc(test_string1);
    history_add_new_entry_to_list(entries, entry);
    expected_older = NULL;

    older_entry = history_get_older_entry_from_list(entries, entry);

    /* check results */
    POINTERS_EQUAL(expected_older, older_entry);
}

TEST(entries, attempt_to_get_newer_than_newest_returns_null)
{
    history_entry_st * entry;
    history_entry_st * newer_entry;
    char const test_string1[] = "test1";
    history_entry_st * expected_newer;

    /* setup */

    /* perform test */
    entry = history_entry_alloc(test_string1);
    history_add_new_entry_to_list(entries, entry);
    expected_newer = NULL;

    newer_entry = history_get_newer_entry_from_list(entries, entry);

    /* check results */
    POINTERS_EQUAL(expected_newer, newer_entry);
}
