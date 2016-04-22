#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>

extern "C"
{
#include "directory.h"
#include "directory_test.h"
#include <sys/stat.h>
#include <stdio.h>
};

int test_stat(const char * file, struct stat * buf)
{
    unsigned int mode;

    mock().actualCall("stat")
    .withParameter("file", file)
    .withOutputParameter("mode", &mode); 

    buf->st_mode = mode;

    return mock().returnLongIntValueOrDefault(-1);

    return 1;
}

TEST_GROUP(directory)
{
    void setup()
    {
    }

    void teardown()
    {
        mock().clear();
    }

    void do_test(int const stat_result, unsigned int const mode, int const expected_is_a_directory)
    {
        int expected_result;
        int actual_result;
        bool is_a_directory;

        /* setup */
        expected_result = 0;
        mock().expectOneCall("stat")
            .withParameter("file", "path/file")
            .withOutputParameterReturning("mode", &mode, sizeof mode)
            .andReturnValue(stat_result);
        /* perform test*/
        actual_result = check_if_path_is_a_directory("path", "file", &is_a_directory);

        /* check results */
        LONGS_EQUAL(expected_result, actual_result);
        CHECK_TRUE(expected_is_a_directory == is_a_directory);
        mock().checkExpectations();
    }
};

TEST(directory, check_path_and_file_passed_and_is_dir)
{
    do_test(0, __S_IFDIR, true);
}

TEST(directory, check_path_and_file_passed_and_not_dir)
{
    do_test(0, 0, false);
}

TEST(directory, failed_stat_returns_not_dir)
{
    do_test(-1, __S_IFDIR, false);
}

