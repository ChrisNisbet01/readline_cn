#include "editline.h"

#include <stdbool.h>
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define HISTORY_SIZE 10

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
    size_t index;
    char const * current_token = completion_context->tokens_get_current_token_fn(completion_context);
    size_t const current_token_len = strlen(current_token);

    for (index = 0; index < NB_ARG0_ITEMS; index++)
    {
        if (strncmp(current_token, arg0_items[index], current_token_len) == 0)
        {
            completion_context->possible_word_add_fn(completion_context, arg0_items[index]);
        }
    }

    return 0;
}

static int test_completion_callback(completion_context_st * const completion_context,
                                    void * const user_context)
{
    size_t index;
    (void)user_context;
    int result = 0;
    size_t current_token_index = completion_context->tokens_get_current_token_index_fn(completion_context); 

    if (current_token_index == 0)
    {
        do_command_name_completion(completion_context);
    }
    else
    {
        result = do_filename_completion(completion_context, user_context);
    }

    return result;
}

static void free_args(size_t argc, char const * * const argv)
{
    if (argv != NULL)
    {
        size_t index;

        for (index = 0; index < argc; index++)
        {
            free((void *)argv[index]);
        }
        free(argv);
    }
}

int main(int argc, char * argv[]__attribute__((unused)))
{
    char * prompt; 
    editline_st * editline_ctx;
    bool do_read_line;

    prompt = "prompt> ";

    editline_ctx = editline_context_create(NULL, 
                                           test_completion_callback, 
                                           STDIN_FILENO, 
                                           STDOUT_FILENO, 
                                           HISTORY_SIZE);

    do
    {
        char * line = NULL;
        size_t argc; 
        char const * * argv = NULL;
        //editline_result_t const result = readline(editline_ctx, 60, prompt, &line);
        editline_result_t const result = readline_args(editline_ctx, 60, prompt, &argc, &argv);

        switch (result)
        {
            case editline_result_success:
            {
                int index;
                
                for (index = 0; index < argc; index++)
                {
                    printf("Got arg %d: '%s'\n", index, argv[index]);
                }
                do_read_line = true;
                    break;
            }
            case editline_result_ctrl_c:
                do_read_line = false;
                printf("Got CTRL-C\n");
                break;
            case editline_result_timed_out:
                do_read_line = false;
                printf("Timed out\n");
                break;
            case editline_result_eof:
                do_read_line = false;
                printf("Got EOF\n");
                break;
            case editline_result_error:
                do_read_line = false;
                printf("Got error\n");
                break;
        }
        free(line);
        free_args(argc, argv);
    }
    while (do_read_line);

    editline_context_destroy(editline_ctx);

    return 0;
}


