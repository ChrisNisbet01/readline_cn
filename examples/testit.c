#include "readline.h"

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
    int characters_printed = 0;
    size_t index;
    char const * current_token = completion_context->tokens_get_current_token_fn(completion_context);
    size_t const current_token_len = strlen(current_token);

    dprintf(completion_context->write_back_fd, "\n  some text from callback\n");
    characters_printed = 1;
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
        result = do_command_name_completion(completion_context);
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

static bool get_password(readline_st * const readline_ctx)
{
    char * line = NULL;
    char const previous_mask_control_character = readline_context_mask_character_control(readline_ctx, '*'); 
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
    readline_context_mask_character_control(readline_ctx, previous_mask_control_character);
}

int main(int argc, char * argv[]__attribute__((unused)))
{
    char * prompt; 
    readline_st * readline_ctx;
    bool do_read_line;

    prompt = "Prompt> ";

    readline_ctx = readline_context_create(NULL, 
                                           test_completion_callback, 
                                           STDIN_FILENO, 
                                           STDOUT_FILENO, 
                                           HISTORY_SIZE);
    if (readline_ctx == NULL)
    {
        fprintf(stderr, "Unable to create readline context\n");
        goto done;
    }
    get_password(readline_ctx);

    do
    {
        size_t argc; 
        char const * * argv = NULL;

        readline_result_t const result = readline_args(readline_ctx, 60, prompt, &argc, &argv);

        switch (result)
        {
            case readline_result_success:
            {
                int index;
                
                for (index = 0; index < argc; index++)
                {
                    printf("Got arg %d: '%s'\n", index, argv[index]);
                }
                do_read_line = true;
                break;
            }
            case readline_result_ctrl_c:
                do_read_line = false;
                printf("Got CTRL-C\n");
                break;
            case readline_result_timed_out:
                do_read_line = false;
                printf("Timed out\n");
                break;
            case readline_result_eof:
            {
                printf("Got EOF\n");
                int index;

                for (index = 0; index < argc; index++)
                {
                    printf("Got arg %d: '%s'\n", index, argv[index]);
                }
                do_read_line = false;
                break;
            }
            case readline_result_error:
                do_read_line = false;
                printf("Got error\n");
                break;
        }
        free_args(argc, argv);
    }
    while (do_read_line);

    readline_context_destroy(readline_ctx);

done:
    return 0;
}


