/* Copyright (C) Chris Nisbet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly 
 * prohibited. Proprietary and confidential. Written by Chris 
 * Nisbet <nisbet@ihug.co.nz>, April 2016.
 */

#include "readline.h"

#include <stdbool.h>
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define HISTORY_SIZE 10
#define UNUSED_PARAMETER(param) (void)param
#define FREE_CONST(const_mem) free((void *)(const_mem))

static char * arg0_items[] =
{
    "def1",
    "abc",
    "def",
    "def3abc",
    "def3abc2"
};
#define NB_ARG0_ITEMS (sizeof(arg0_items)/sizeof(arg0_items[0]))

static int do_command_name_completion(completion_context_st * const completion_context)
{
    int characters_printed = 0;
    size_t index;
    char const * current_token = completion_context->tokens_get_current_token_fn(completion_context);
    size_t const current_token_len = strlen(current_token);
    size_t num_possible_words;

    for (index = 0, num_possible_words = 0; index < NB_ARG0_ITEMS; index++)
    {
        if (strncmp(current_token, arg0_items[index], current_token_len) == 0)
        {
            completion_context->possible_word_add_fn(completion_context, arg0_items[index]);
            num_possible_words++;
        }
    }
    if (num_possible_words != 1)
    {
        dprintf(completion_context->write_back_fd, "\nsome text from callback\n");
        characters_printed = 1;
    }

    return characters_printed;
}

static int test_completion_callback(completion_context_st * const completion_context,
                                    void * const user_context)
{
    int result = 0;
    size_t const current_token_index = completion_context->tokens_get_current_token_index_fn(completion_context); 

    UNUSED_PARAMETER(user_context);

    if (current_token_index == 0)
    {
        result = do_command_name_completion(completion_context);
    }
    else
    {
        result = do_filename_completion(completion_context, user_context);
    }

    return result;
}

static int test_help_callback(help_context_st * const help_context,
                                    void * const user_context)
{
    int result = 1;
    size_t const current_token_index = help_context->tokens_get_current_token_index_fn(help_context);

    UNUSED_PARAMETER(user_context);

    dprintf(help_context->write_back_fd, "\nhere's some help and current token_index %zu\n", current_token_index);

    return result;
}

static void free_args(size_t argc, char const * const * const argv)
{
    if (argv != NULL)
    {
        size_t index;

        for (index = 0; index < argc; index++)
        {
            FREE_CONST(argv[index]);
        }
        FREE_CONST(argv);
    }
}

static void do_print_args(int const argc, char const * const * const argv)
{
    int index;

    for (index = 0; index < argc; index++)
    {
        printf("Got arg %d: '%s'\n", index, argv[index]);
    }
}

static bool get_password(readline_st * const readline_ctx)
{
    char * line = NULL;
    char const previous_mask_control_character = readline_set_mask_character(readline_ctx, '*');
    readline_result_t const result = readline(readline_ctx, 60, "Password> ", &line);
    bool got_password;

    if (result == readline_result_success || result == readline_result_eof)
    {
        printf("Got password: '%s'\n", line);
        got_password = true;
    }
    else
    {
        got_password = false; 
    }
    free(line);
    readline_set_mask_character(readline_ctx, previous_mask_control_character);

    return got_password;
}

static bool process_readline_result(readline_result_t const readline_result, int const argc, char const * const * const argv)
{
    bool continue_processing;
    bool print_args;

    switch (readline_result)
    {
        case readline_result_success:
        {
            print_args = true;
            continue_processing = true;
            break;
        }
        case readline_result_ctrl_c:
            printf("Got CTRL-C\n");
            print_args = false;
            continue_processing = false;
            break;
        case readline_result_timed_out:
            printf("Timed out\n");
            print_args = false;
            continue_processing = false;
            break;
        case readline_result_eof:
        {
            printf("Got EOF\n");
            print_args = true;
            continue_processing = false;
            break;
        }
        case readline_result_error:
            printf("Got error\n");
            print_args = false;
            continue_processing = false;
            break;
        default:
            print_args = false;
            continue_processing = false;
    }

    if (print_args)
    {
        do_print_args(argc, argv);
    }

    return continue_processing;
}

static bool read_line(readline_st * const readline_ctx, char const * const prompt)
{
    bool continue_processing;
    size_t readline_argc;
    char const * * readline_argv = NULL;

    readline_result_t const result = readline_args(readline_ctx, 60, prompt, &readline_argc, &readline_argv);

    continue_processing = process_readline_result(result, readline_argc, readline_argv);

    free_args(readline_argc, readline_argv);

    return continue_processing;
}

static void read_lines(readline_st * const readline_ctx)
{
    bool continue_processing;
    char const prompt[] = "Prompt> ";

    readline_set_field_separators(readline_ctx, "|");
    readline_set_maximum_line_length(readline_ctx, 256);

    do
    {
        continue_processing = read_line(readline_ctx, prompt);
    }
    while (continue_processing);
}

int main(int argc, char * * argv)
{
    readline_st * readline_ctx;

    UNUSED_PARAMETER(argc);
    UNUSED_PARAMETER(argv);

    readline_ctx = readline_context_create(NULL, 
                                           test_completion_callback,
                                           test_help_callback,
                                           '?',
                                           STDIN_FILENO,
                                           STDOUT_FILENO, 
                                           HISTORY_SIZE);
    if (readline_ctx == NULL)
    {
        fprintf(stderr, "Unable to create readline context\n");
        goto done;
    }
    get_password(readline_ctx);
    read_lines(readline_ctx);

done:
    readline_context_destroy(readline_ctx);

    return 0;
}


