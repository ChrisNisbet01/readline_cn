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

int isatty(int fd)
{
    mock().actualCall("isatty");

    return mock().returnIntValueOrDefault(0);;
}

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

    close(stdout_fd); 

    STRCMP_EQUAL(expected_result, line);

    free(line);
    readline_context_destroy(readline_ctx);

    LONGS_EQUAL(readline_result_success, result);
    /* check results */
    mock().checkExpectations();
}

TEST(readline, simple_string_no_tty)
{
    int stdin_pipe[2];
    int stdout_pipe[2];

    pipe(stdin_pipe);
    pipe(stdout_pipe);

    dprintf(stdin_pipe[1], "1234\n");

    mock().disable();
    child_process(stdin_pipe[0], stdout_pipe[1], "1234");
}

TEST(readline, empty_string_no_tty)
{
    int stdin_pipe[2];
    int stdout_pipe[2];

    pipe(stdin_pipe);
    pipe(stdout_pipe);

    dprintf(stdin_pipe[1], "\n");
    mock().disable();
    child_process(stdin_pipe[0], stdout_pipe[1], "");
}

static void write_control_sequence(int const fd, char control_char)
{
    dprintf(fd, "%c", CTL(control_char));
}

static void write_home_sequence(int const fd)
{
    write_control_sequence(fd, 'A');
}

static void write_end_sequence(int const fd)
{
    write_control_sequence(fd, 'E');
}

static void write_basic_escape_sequence(int const fd, char escape_char)
{
    dprintf(fd, "\033[%c", escape_char);
}

static void write_right_arrow_sequence(int const fd)
{
    write_basic_escape_sequence(fd, 'C');
}

static void write_left_arrow_sequence(int const fd)
{
    write_basic_escape_sequence(fd, 'D');
}

static void write_1_semi_5_escape_sequence(int const fd, char const escape_char)
{
    dprintf(fd, "\033[1;5%c", escape_char);
}

static void write_control_left_arrow_sequence(int const fd)
{
    write_1_semi_5_escape_sequence(fd, 'D');
}

static void write_control_right_arrow_sequence(int const fd)
{
    write_1_semi_5_escape_sequence(fd, 'C');
}

TEST(readline, move_home_and_add_char)
{
    int stdin_pipe[2];
    int stdout_pipe[2];

    /* setup */
    pipe(stdin_pipe);
    pipe(stdout_pipe);
    mock().expectOneCall("isatty").andReturnValue(1);
    dprintf(stdin_pipe[1], "1234");
    write_home_sequence(stdin_pipe[1]); 
    dprintf(stdin_pipe[1], "a"); 
    dprintf(stdin_pipe[1], "\n"); 

    /* perform test */
    child_process(stdin_pipe[0], stdout_pipe[1], "a1234");

}

TEST(readline, move_home_and_add_char_then_move_end_and_add_char)
{
    int stdin_pipe[2];
    int stdout_pipe[2];

    /* setup */
    pipe(stdin_pipe);
    pipe(stdout_pipe);
    mock().expectOneCall("isatty").andReturnValue(1);
    dprintf(stdin_pipe[1], "1234");
    write_home_sequence(stdin_pipe[1]);
    dprintf(stdin_pipe[1], "a");
    write_end_sequence(stdin_pipe[1]); 
    dprintf(stdin_pipe[1], "b"); 

    dprintf(stdin_pipe[1], "\n");

    /* perform test */
    child_process(stdin_pipe[0], stdout_pipe[1], "a1234b");

}

TEST(readline, move_left_one_char_and_add)
{
    int stdin_pipe[2];
    int stdout_pipe[2];

    /* setup */
    pipe(stdin_pipe);
    pipe(stdout_pipe);
    mock().expectOneCall("isatty").andReturnValue(1);
    dprintf(stdin_pipe[1], "1234");
    write_left_arrow_sequence(stdin_pipe[1]);
    dprintf(stdin_pipe[1], "b");

    dprintf(stdin_pipe[1], "\n");

    /* perform test */
    child_process(stdin_pipe[0], stdout_pipe[1], "123b4");

}

TEST(readline, move_left_one_char_and_add_then_right_one_char_and_add)
{
    int stdin_pipe[2];
    int stdout_pipe[2];

    /* setup */
    pipe(stdin_pipe);
    pipe(stdout_pipe);
    mock().expectOneCall("isatty").andReturnValue(1);
    dprintf(stdin_pipe[1], "1234");
    write_left_arrow_sequence(stdin_pipe[1]);
    dprintf(stdin_pipe[1], "a");
    write_right_arrow_sequence(stdin_pipe[1]);
    dprintf(stdin_pipe[1], "b"); 

    dprintf(stdin_pipe[1], "\n");

    /* perform test */
    child_process(stdin_pipe[0], stdout_pipe[1], "123a4b");

}

TEST(readline, move_left_one_word_and_add)
{
    int stdin_pipe[2];
    int stdout_pipe[2];

    /* setup */
    pipe(stdin_pipe);
    pipe(stdout_pipe);
    mock().expectOneCall("isatty").andReturnValue(1);
    dprintf(stdin_pipe[1], "abc def hij");
    write_control_left_arrow_sequence(stdin_pipe[1]);
    dprintf(stdin_pipe[1], "1");

    dprintf(stdin_pipe[1], "\n");

    /* perform test */
    child_process(stdin_pipe[0], stdout_pipe[1], "abc def 1hij");
}

TEST(readline, move_left_two_words_and_add)
{
    int stdin_pipe[2];
    int stdout_pipe[2];

    /* setup */
    pipe(stdin_pipe);
    pipe(stdout_pipe);
    mock().expectOneCall("isatty").andReturnValue(1);
    dprintf(stdin_pipe[1], "abc def hij");
    write_control_left_arrow_sequence(stdin_pipe[1]);
    write_control_left_arrow_sequence(stdin_pipe[1]);
    dprintf(stdin_pipe[1], "1");

    dprintf(stdin_pipe[1], "\n");

    /* perform test */
    child_process(stdin_pipe[0], stdout_pipe[1], "abc 1def hij");
}

TEST(readline, move_left_two_words_and_add_then_right_one_and_add)
{
    int stdin_pipe[2];
    int stdout_pipe[2];

    /* setup */
    pipe(stdin_pipe);
    pipe(stdout_pipe);
    mock().expectOneCall("isatty").andReturnValue(1);
    dprintf(stdin_pipe[1], "abc def hij");
    write_control_left_arrow_sequence(stdin_pipe[1]);
    write_control_left_arrow_sequence(stdin_pipe[1]);
    dprintf(stdin_pipe[1], "1");
    write_control_right_arrow_sequence(stdin_pipe[1]);
    dprintf(stdin_pipe[1], "2"); 

    dprintf(stdin_pipe[1], "\n");

    /* perform test */
    child_process(stdin_pipe[0], stdout_pipe[1], "abc 1def2 hij");
}

TEST(readline, move_right_words_when_already_at_rightmost_has_no_effect)
{
    int stdin_pipe[2];
    int stdout_pipe[2];

    /* setup */
    pipe(stdin_pipe);
    pipe(stdout_pipe);
    mock().expectOneCall("isatty").andReturnValue(1);
    dprintf(stdin_pipe[1], "abc def hij");
    write_control_right_arrow_sequence(stdin_pipe[1]);
    dprintf(stdin_pipe[1], "2");

    dprintf(stdin_pipe[1], "\n");

    /* perform test */
    child_process(stdin_pipe[0], stdout_pipe[1], "abc def hij2");
}

