/* Copyright (C) Chris Nisbet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly 
 * prohibited. Proprietary and confidential. Written by Chris 
 * Nisbet <nisbet@ihug.co.nz>, April 2016.
 */

#include <CppUTest/TestHarness.h>

extern "C"
{
#include "split_path.h"
};

#include <string.h>

TEST_GROUP(split_path)
{
    char const * dir_part;
    char const * file_part;
    size_t completion_start_index;

    void setup()
    {
        dir_part = NULL;
        file_part = NULL;
    }

    void teardown()
    {
        free((void *)dir_part);
        free((void *)file_part);
    }
};

TEST(split_path, split_empty_string)
{
    char expected_dir_path[] = ".";
    char expected_file_part[] = "";
    char const path_to_split[] = "";

    /* setup */

    /* perform test */
    (void)split_path(path_to_split, &dir_part, &file_part, &completion_start_index);

    /* check result */
    STRCMP_EQUAL(expected_dir_path, dir_part);
    STRCMP_EQUAL(expected_file_part, file_part);
    LONGS_EQUAL(0, completion_start_index);
}

TEST(split_path, split_no_dir_part)
{
    char expected_dir_path[] = ".";
    char expected_file_part[] = "abc";
    char const path_to_split[] = "abc";

    /* setup */

    /* perform test */
    (void)split_path(path_to_split, &dir_part, &file_part, &completion_start_index); 

    /* check result */
    STRCMP_EQUAL(expected_dir_path, dir_part);
    STRCMP_EQUAL(expected_file_part, file_part);
    LONGS_EQUAL(0, completion_start_index);
}

TEST(split_path, split_all_dir_part)
{
    char expected_dir_path[] = "abc/";
    char expected_file_part[] = "";
    char const path_to_split[] = "abc/";

    /* setup */

    /* perform test */
    (void)split_path(path_to_split, &dir_part, &file_part, &completion_start_index); 

    /* check result */
    STRCMP_EQUAL(expected_dir_path, dir_part);
    STRCMP_EQUAL(expected_file_part, file_part);
    LONGS_EQUAL(4, completion_start_index);
}

TEST(split_path, split_dir_and_file)
{
    char expected_dir_path[] = "abc/";
    char expected_file_part[] = "def";
    char const path_to_split[] = "abc/def";

    /* setup */

    /* perform test */
    (void)split_path(path_to_split, &dir_part, &file_part, &completion_start_index); 

    /* check result */
    STRCMP_EQUAL(expected_dir_path, dir_part);
    STRCMP_EQUAL(expected_file_part, file_part);
    LONGS_EQUAL(4, completion_start_index);
}

