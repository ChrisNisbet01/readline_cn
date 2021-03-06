/* Copyright (C) Chris Nisbet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly 
 * prohibited. Proprietary and confidential. Written by Chris 
 * Nisbet <nisbet@ihug.co.nz>, April 2016.
 */

#include "word_completion.h"
#include "tokenise.h"
#include "args.h"
#include "strdup_partial.h"
#include "common_prefix_length.h"
#include "print_words_in_columns.h"
#include "readline_context.h"
#include "utils.h"

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define GET_PRIVATE_CONTEXT_FROM_PUBLIC_CONTEXT(completion_context) \
    container_of(completion_context, private_completion_context_st, public_context)

static char const * find_longest_completion_suffix(size_t const token_length, size_t const num_words, char const * const * const words)
{
    char const * longest_match;

    /* given a token length and a set of possible matches, find the
     * longest match from the set of words. 
     * An example might be the easiest way to explain.
     * e.g. if token is "a" and words is "abc", "abd", return "ab". 
     * It is assumed that all words match up to token_length. 
     */
    if (num_words == 0)
    {
        longest_match = NULL;
    }
    else if (num_words == 1)
    {
        longest_match = strdup_partial(words[0], token_length, strlen(words[0]));
    }
    else
    {
        size_t const common_prefix_length = find_common_prefix_length(num_words, words);

        /* Multiple tokens. Find the longest common prefix shared by 
         * the words. 
         */
        if (token_length == 0)
        {
            longest_match = NULL;
            goto done;
        }
        if (common_prefix_length <= token_length)
        {
            longest_match = NULL;
            goto done;
        }
        longest_match = strdup_partial(words[0], token_length, common_prefix_length);
    }

done:
    return longest_match;
}

/*
 * possible_word_add. 
 * Called by the user code when running the completion callback. 
 * Adds the supplied word into the list of possible options. 
*/
static int possible_word_add(completion_context_st * const completion_context,
                             char const * const possible_word)
{
    private_completion_context_st * const private_completion_context =
        GET_PRIVATE_CONTEXT_FROM_PUBLIC_CONTEXT(completion_context);
    args_st * possible_words;

    possible_words = private_completion_context->possible_words;

    args_add_arg(possible_words, possible_word);

    return 0;
}

static void set_completion_start(completion_context_st * const completion_context,
                                 size_t completion_start_index)
{
    private_completion_context_st * const private_completion_context =
        GET_PRIVATE_CONTEXT_FROM_PUBLIC_CONTEXT(completion_context);
    tokens_st * const tokens = private_completion_context->tokens;

    /* completion_start_index is _not_ an index within the line; 
     * it is an index within the current token, so should be in the 
     * range 0 -> <current token length>.
     */
    if (tokens_index_is_within_current_token(tokens, completion_start_index))
    {
        private_completion_context->completion_start_index = completion_start_index;
    }

}

static int unique_match_set(completion_context_st * const completion_context,
                            char const * const unique_match)
{
    private_completion_context_st * const private_completion_context =
        GET_PRIVATE_CONTEXT_FROM_PUBLIC_CONTEXT(completion_context);

    FREE_CONST(private_completion_context->unique_match);
    private_completion_context->unique_match = strdup(unique_match);

    return 0;
}

static size_t get_current_token_index(completion_context_st * const completion_context)
{
    private_completion_context_st * const private_completion_context =
        GET_PRIVATE_CONTEXT_FROM_PUBLIC_CONTEXT(completion_context);

    return tokens_get_current_token_index(private_completion_context->tokens);
}

static char const * get_current_token(completion_context_st * const completion_context)
{
    private_completion_context_st * const private_completion_context =
        GET_PRIVATE_CONTEXT_FROM_PUBLIC_CONTEXT(completion_context);

    return tokens_get_current_token(private_completion_context->tokens);
}

static size_t get_num_tokens(completion_context_st * const completion_context)
{
    private_completion_context_st * const private_completion_context =
        GET_PRIVATE_CONTEXT_FROM_PUBLIC_CONTEXT(completion_context);

    return tokens_get_num_tokens(private_completion_context->tokens);
}

static char const * get_token_at_index(completion_context_st * const completion_context, size_t const index)
{
    private_completion_context_st * const private_completion_context =
        GET_PRIVATE_CONTEXT_FROM_PUBLIC_CONTEXT(completion_context);

    return tokens_get_token_at_index(private_completion_context->tokens, index);
}

static int qsort_string_compare(const void * p1, const void * p2)
{
    char const * const * const v1 = (char const * const *)p1;
    char const * const * const v2 = (char const * const *)p2;

    return strcmp(*v1, *v2);
}

static void private_completion_context_teardown(private_completion_context_st * const private_completion_context)
{
    completion_context_st * const completion_context = &private_completion_context->public_context;

    if (completion_context->write_back_fd != -1)
    {
        close(completion_context->write_back_fd);
        completion_context->write_back_fd = -1;
    }
    
    args_free(private_completion_context->possible_words);
    private_completion_context->possible_words = NULL;
    tokens_free(private_completion_context->tokens);
    private_completion_context->tokens = NULL;
}

static bool private_completion_context_init(line_context_st * const line_ctx,
                                            private_completion_context_st * const private_completion_context,
                                            char const * const field_separators)
{
    completion_context_st * const completion_context = &private_completion_context->public_context;
    bool init_ok;

    memset(private_completion_context, 0, sizeof *private_completion_context);
    private_completion_context->possible_words = args_alloc(1);

    if (private_completion_context->possible_words == NULL)
    {
        init_ok = false;
        goto done;
    }
    private_completion_context->tokens = tokenise_line(line_ctx->edit_buffer, 0, line_ctx->edit_index, true, field_separators);
    if (private_completion_context->tokens == NULL)
    {
        init_ok = false;
        goto done;
    }

    /* dup() the file descriptor because it will be closed during 
     * completion. 
     */
    /* XXX - Do error checking. */
    completion_context->write_back_fd = dup(line_ctx->terminal_fd);

    completion_context->possible_word_add_fn = possible_word_add;
    completion_context->start_index_set_fn = set_completion_start;
    completion_context->unique_match_set_fn = unique_match_set;
    completion_context->tokens_get_current_token_index_fn = get_current_token_index;
    completion_context->tokens_get_current_token_fn = get_current_token;
    completion_context->tokens_get_num_tokens_fn = get_num_tokens;
    completion_context->tokens_get_token_at_index_fn = get_token_at_index;

    init_ok = true;

done:
    if (!init_ok)
    {
        private_completion_context_teardown(private_completion_context);
    }
    return init_ok;
}

static void process_unique_match(private_completion_context_st * const private_completion_context,
                                 line_context_st * const line_ctx)
{
    /* This field is currently set by the filename completion 
     * callback. It is used when the word to be displayed doesn't 
     * exactly match the single matching word. 
     * i.e. it will have a '/' or ' ' appended to it if it 
     * represents a directory or filename respectively. 
     */
    char const * longest_completion_suffix;
    char const * const current_token = tokens_get_current_token(private_completion_context->tokens);
    char const * const token = &current_token[private_completion_context->completion_start_index];

    longest_completion_suffix = find_longest_completion_suffix(strlen(token),
                                                               1,
                                                               &private_completion_context->unique_match);
    if (longest_completion_suffix != NULL)
    {
        complete_word(line_ctx, longest_completion_suffix, true);
        FREE_CONST(longest_completion_suffix);
    }
}

static bool process_multiple_matches(line_context_st * const line_ctx,
                                     private_completion_context_st * const private_completion_context)
{
    bool need_to_redisplay_line;

    /* The line will need to be redisplayed if any possible options 
     * are printed onto the display. It should also be redisplayed 
     * if the user chose to write to the write_back_fd file 
     * descriptor, but we're relying on him to inform us that this 
     * has been done. I'm not sure how I can figure out if he's 
     * written to the descriptor without the user specifically 
     * letting us know. 
     */
    need_to_redisplay_line = false;

    if (private_completion_context->possible_words->argc > 0)
    {
        char const * longest_completion_suffix;
        bool printed_additional_lines;
        char const * const current_token = tokens_get_current_token(private_completion_context->tokens);
        char const * const token = &current_token[private_completion_context->completion_start_index];

        printed_additional_lines = false;
        if (private_completion_context->possible_words->argc > 1)
        {
            qsort(private_completion_context->possible_words->argv,
                  private_completion_context->possible_words->argc, sizeof(*private_completion_context->possible_words->argv),
                  qsort_string_compare);
            print_words_in_columns(line_ctx->terminal_fd,
                                   line_ctx->terminal_width,
                                   private_completion_context->possible_words->argc,
                                   private_completion_context->possible_words->argv);
            need_to_redisplay_line = true;
            printed_additional_lines = true;
        }

        longest_completion_suffix = find_longest_completion_suffix(strlen(token),
                                                                   private_completion_context->possible_words->argc,
                                                                   private_completion_context->possible_words->argv);
        if (longest_completion_suffix != NULL)
        {
            complete_word(line_ctx,
                          longest_completion_suffix,
                          !printed_additional_lines);
            FREE_CONST(longest_completion_suffix);
        }
    }
    return need_to_redisplay_line;
}

static void private_completion_context_process_results(line_context_st * const line_ctx,
                                                       private_completion_context_st * const private_completion_context,
                                                       bool const characters_were_printed)
{
    bool need_to_redisplay_line;

    if (private_completion_context->unique_match != NULL)
    {
        process_unique_match(private_completion_context, line_ctx);
        need_to_redisplay_line = false;
    }
    else
    {
        need_to_redisplay_line = process_multiple_matches(line_ctx, private_completion_context);
    }

    if (characters_were_printed)
    {
        need_to_redisplay_line = true;
    }

    if (need_to_redisplay_line)
    {
        redisplay_line(line_ctx);
    }
}

void do_word_completion(readline_st * const readline_ctx)
{
    int characters_were_printed;

    line_context_st * const line_ctx = &readline_ctx->line_context;
    private_completion_context_st private_completion_context;

    /* Word completion isn't performed if the user has requested 
     * to mask the input characters. 
     */
    if (readline_ctx->mask_character != '\0')
    {
        goto done;
    }
    if (readline_ctx->completion_callback == NULL)
    {
        goto done;
    }

    if (!private_completion_context_init(line_ctx,
                                         &private_completion_context,
                                         readline_ctx->field_separators))
    {
        goto done;
    }

    characters_were_printed = readline_ctx->completion_callback(&private_completion_context.public_context,
                                      readline_ctx->user_context);

    private_completion_context_process_results(line_ctx,
                                               &private_completion_context,
                                               characters_were_printed > 0);

    private_completion_context_teardown(&private_completion_context);

done:
    return;
}


