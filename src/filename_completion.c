/* Copyright (C) Chris Nisbet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly 
 * prohibited. Proprietary and confidential. Written by Chris 
 * Nisbet <nisbet@ihug.co.nz>, April 2016.
 */

#include "readline.h"
#include "directory.h"
#include "split_path.h"
#include "utils.h"

#include <dirent.h>
#include <string.h>
#include <stdlib.h>

static void process_filename(completion_context_st * const completion_context, 
                             char const * const filename, 
                             char const * const filename_token,
                             size_t const filename_token_len)
{
    if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0)
    {
        goto done;
    }

    /* check for a partial match. */
    if (filename_token_len > 0 && strncmp(filename, filename_token, filename_token_len) != 0)
    {
        goto done;
    }

    completion_context->possible_word_add_fn(completion_context, filename);

done:
    return;
}

static int process_unique_match(completion_context_st * const completion_context, 
                                char const * const directory_name,
                                char const * const filename)
{
    int result;
    char * unique_match = NULL;
    char char_to_append;
    bool is_a_directory;

    if (check_if_path_is_a_directory(directory_name, filename, &is_a_directory) < 0)
    {
        result = -1;
        goto done;
    }

    if (is_a_directory)
    {
        char_to_append = '/';
    }
    else
    {
        char_to_append = ' ';
    }

    if (asprintf(&unique_match, "%s%c", filename, char_to_append) < 0)
    {
        goto done;
    }
    completion_context->unique_match_set_fn(completion_context, unique_match);
    free(unique_match);
    result = 0;

done:
    return result;
}

static int do_complete_filename_matches(completion_context_st * const completion_context,
                                        char const * const directory_name, 
                                        char const * const filename_token)
{
    private_completion_context_st * const private_completion_context =
        (private_completion_context_st *)((char *)completion_context - offsetof(private_completion_context_st, public_context));
    int result;
    DIR * directory;
    struct dirent * directory_entry;
    size_t const filename_token_len = strlen(filename_token);

    directory = opendir(directory_name);
    if (directory == NULL)
    {
        result = -1;
        goto done;
    }

    while ((directory_entry = readdir(directory)) != NULL)
    {
        char const * filename = directory_entry->d_name;

        process_filename(completion_context, filename, filename_token, filename_token_len);
    }

    /* If there is a single match append either a slash if the match
     * is a directory, else append a space. 
     */
    if (private_completion_context->possible_words->argc == 1)
    {
        process_unique_match(completion_context, directory_name, private_completion_context->possible_words->argv[0]);
    }

    result = 0;

done:
    if (directory != NULL)
    {
        closedir(directory);
    }

    return result;
}

int do_filename_completion(completion_context_st * const completion_context,
                           void * const user_context)
{
    char const * dir = NULL;
    char const * file = NULL;
    (void)user_context;
    size_t completion_start_index = 0;
    char const * current_token = completion_context->tokens_get_current_token_fn(completion_context);

    if (split_path(current_token, &dir, &file, &completion_start_index) < 0)
    {
        goto done;
    }

    completion_context->start_index_set_fn(completion_context, completion_start_index);

    do_complete_filename_matches(completion_context, dir, file);

    FREE_CONST(dir);
    FREE_CONST(file);

done:
    return 0;
}


