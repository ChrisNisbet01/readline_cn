/* Copyright (C) Chris Nisbet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly 
 * prohibited. Proprietary and confidential. Written by Chris 
 * Nisbet <nisbet@ihug.co.nz>, April 2016.
 */

#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>

#include <unistd.h>
#include <stdio.h>
extern "C"
{
#include "readline.h"
};

TEST_GROUP(readline)
{
    void setup()
    {
    }

    void teardown()
    {
        mock().clear();
    }

};

static void child_process(int const stdin_fd, int const stdout_fd, char const * const expected_result)
{
    readline_st * readline_ctx;
    readline_result_t result;
    char * line;

    readline_ctx = readline_context_create(NULL,
                                           NULL,
                                           NULL,
                                           '\0',
                                           stdin_fd,
                                           stdout_fd,
                                           0);
    CHECK(readline_ctx != NULL);

    result = readline(readline_ctx, 0, "", &line);

    STRCMP_EQUAL(expected_result, line);

    free(line);
    readline_context_destroy(readline_ctx);

    LONGS_EQUAL(readline_result_success, result);
    close(stdout_fd);
}

TEST(readline, simple_string_no_tty)
{
    int stdin_pipe[2];
    int stdout_pipe[2];

    pipe(stdin_pipe);
    pipe(stdout_pipe);

    dprintf(stdin_pipe[1], "1234\n");
    child_process(stdin_pipe[0], stdout_pipe[1], "1234");
}

TEST(readline, empty_string_no_tty)
{
    int stdin_pipe[2];
    int stdout_pipe[2];

    pipe(stdin_pipe);
    pipe(stdout_pipe);

    dprintf(stdin_pipe[1], "\n");
    child_process(stdin_pipe[0], stdout_pipe[1], "");
}

