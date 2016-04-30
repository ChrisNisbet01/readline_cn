/* Copyright (C) Chris Nisbet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly 
 * prohibited. Proprietary and confidential. Written by Chris 
 * Nisbet <nisbet@ihug.co.nz>, April 2016.
 */

#include <CppUTest/TestHarness.h>

extern "C"
{
#include "strdup_partial.h"
};

#include <string.h>

TEST_GROUP(strdup_partial)
{
    char * result;

    void setup()
    {
        result = NULL;
    }

    void teardown()
    {
        free(result);
    }
};

TEST(strdup_partial, null_source_with_bytes_to_copy_returns_null)
{
    char * expected_result;
    char * source;

    /* setup */
    expected_result = NULL;
    source = NULL;

    /* perform test */
    result = strdup_partial(source, 1, 2);

    /* check results */
    POINTERS_EQUAL(expected_result, result);
}

TEST(strdup_partial, null_source_with_no_bytes_to_copy_returns_empty_string)
{
    char * expected_result;
    char * source;
    char empty_string[] = "";

    /* setup */
    expected_result = empty_string;
    source = NULL;

    /* perform test */
    result = strdup_partial(source, 1, 1);

    /* check results */
    STRCMP_EQUAL(expected_result, result);
}

TEST(strdup_partial, end_lt_start_returns_null)
{
    char * expected_result;
    char test_source[] = "123";
    char * source;

    /* setup */
    expected_result = NULL;
    source = test_source;

    /* perform test */
    result = strdup_partial(source, 2, 1);

    /* check results */
    POINTERS_EQUAL(expected_result, result);
}

TEST(strdup_partial, end_gt_source_len_returns_null)
{
    char * expected_result;
    char test_source[] = "123";
    char * source;

    /* setup */
    expected_result = NULL;
    source = test_source;

    /* perform test */
    result = strdup_partial(source, 2, strlen(source) + 1);

    /* check results */
    POINTERS_EQUAL(expected_result, result);
}

TEST(strdup_partial, start_gt_source_len_returns_null)
{
    char * expected_result;
    char test_source[] = "123";
    char * source;

    /* setup */
    expected_result = NULL;
    source = test_source;

    /* perform test */
    result = strdup_partial(source, strlen(source) + 1, 2);

    /* check results */
    POINTERS_EQUAL(expected_result, result);
}

TEST(strdup_partial, start_eq_end_returns_empty_string)
{
    char * expected_result;
    char test_source[] = "123";
    char empty_string[] = "";
    char * source;

    /* setup */
    expected_result = empty_string;
    source = test_source;

    /* perform test */
    result = strdup_partial(source, 2, 2);

    /* check results */
    STRCMP_EQUAL(expected_result, result);
}

TEST(strdup_partial, start_lt_end_returns_partial_string)
{
    char * expected_result;
    char test_source[] = "123";
    char partial_string[] = "12";
    char * source;

    /* setup */
    expected_result = partial_string;
    source = test_source;

    /* perform test */
    result = strdup_partial(source, 0, 2);

    /* check results */
    STRCMP_EQUAL(expected_result, result);
}

TEST(strdup_partial, non_zero_start_lt_end_returns_partial_string)
{
    char * expected_result;
    char test_source[] = "123";
    char partial_string[] = "23";
    char * source;

    /* setup */
    expected_result = partial_string;
    source = test_source;

    /* perform test */
    result = strdup_partial(source, 1, 3);

    /* check results */
    STRCMP_EQUAL(expected_result, result);
}

