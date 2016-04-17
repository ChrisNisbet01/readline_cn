#include "word_completion.h"
#include "tokenise.h"
#include "args.h"
#include "strdup_partial.h"
#include "common_prefix_length.h"
#include "print_words_in_columns.h"
#include "readline_context.h"
#include "terminal.h"

#include <stddef.h>
#include <string.h>
#include <stdlib.h>

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:        the pointer to the member.
 * @type:       the type of the container struct this is embedded in.
 * @member:     the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})

#define GET_PRIVATE_CONTEXT_FROM_PUBLIC_CONTEXT(completion_context) \
    container_of(completion_context, private_completion_context_st, public_context)

static char       newline_str[] = "\n"; 

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

    // TODO - Validate completion_start_index.
    private_completion_context->completion_start_index = completion_start_index;

}

static int unique_match_set(completion_context_st * const completion_context,
                            char const * const unique_match)
{
    private_completion_context_st * const private_completion_context =
        GET_PRIVATE_CONTEXT_FROM_PUBLIC_CONTEXT(completion_context);

    free((void *)private_completion_context->unique_match);
    private_completion_context->unique_match = strdup(unique_match);

    return 0;
}

size_t get_current_token_index(completion_context_st * const completion_context)
{
    private_completion_context_st * const private_completion_context =
        GET_PRIVATE_CONTEXT_FROM_PUBLIC_CONTEXT(completion_context);

    return tokens_get_current_token_index(private_completion_context->tokens);
}

char const * get_current_token(completion_context_st * const completion_context)
{
    private_completion_context_st * const private_completion_context =
        GET_PRIVATE_CONTEXT_FROM_PUBLIC_CONTEXT(completion_context);

    return tokens_get_current_token(private_completion_context->tokens);
}

size_t get_num_tokens(completion_context_st * const completion_context)
{
    private_completion_context_st * const private_completion_context =
        GET_PRIVATE_CONTEXT_FROM_PUBLIC_CONTEXT(completion_context);

    return tokens_get_num_tokens(private_completion_context->tokens);
}

char const * get_token_at_index(completion_context_st * const completion_context, size_t const index)
{
    private_completion_context_st * const private_completion_context =
        GET_PRIVATE_CONTEXT_FROM_PUBLIC_CONTEXT(completion_context);

    return tokens_get_token_at_index(private_completion_context->tokens, index);
}

static int qsort_string_compare(const void * p1, const void * p2)
{
    char const * const * const v1 = (char const * const *)p1;
    char const * const * const v2 = (char const * const *)p2;

    return strcmp( *v1, *v2);
}

static void private_completion_context_teardown(private_completion_context_st * const private_completion_context)
{
    completion_context_st * const completion_context = &private_completion_context->public_context;

    if (completion_context->write_back_fp != NULL)
    {
        fclose(completion_context->write_back_fp);
        completion_context->write_back_fp = NULL;
    }
    free(private_completion_context->freeform_text);
    private_completion_context->freeform_text = NULL;
    args_free(private_completion_context->possible_words);
    private_completion_context->possible_words = NULL;
    tokens_free(private_completion_context->tokens);
    private_completion_context->tokens = NULL;
}

static bool private_completion_context_init(private_completion_context_st * const private_completion_context, line_context_st * const line_ctx)
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
    private_completion_context->tokens = tokenise_line(line_ctx->buffer, 0, line_ctx->cursor_index, true);
    if (private_completion_context->tokens == NULL)
    {
        init_ok = false;
        goto done;
    }

    completion_context->write_back_fp = open_memstream(&private_completion_context->freeform_text,
                                                       &private_completion_context->freeform_text_len);
    if (completion_context->write_back_fp == NULL)
    {
        init_ok = false;
        goto done;
    }

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
        free((void *)longest_completion_suffix);
    }
}

static void process_multiple_matches(private_completion_context_st * const private_completion_context,
                                     readline_st * const readline_ctx)
{
    line_context_st * const line_ctx = &readline_ctx->line_context;
    completion_context_st * const completion_context = &private_completion_context->public_context;
    bool need_to_redisplay_line;

    /* The line will need to be redisplayed if any possible 
     * options or free text are printed onto the display. 
     */
    need_to_redisplay_line = false;

    fclose(completion_context->write_back_fp);
    completion_context->write_back_fp = NULL;
    if (private_completion_context->freeform_text != NULL && private_completion_context->freeform_text_len > 0)
    {
        tty_puts(readline_ctx->out_fd, newline_str, '\0');
        tty_puts(readline_ctx->out_fd, private_completion_context->freeform_text, '\0');
        need_to_redisplay_line = true;
    }

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
                  private_completion_context->possible_words->argc, sizeof( *private_completion_context->possible_words->argv),
                  qsort_string_compare);
            print_words_in_columns(readline_ctx->out_fd,
                                   readline_ctx->terminal_width,
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
            free((void *)longest_completion_suffix);
        }
    }
    if (need_to_redisplay_line)
    {
        tty_puts(line_ctx->terminal_fd, newline_str, '\0');
        redisplay_line(line_ctx, readline_ctx->prompt);
    }
}

static void private_completion_context_process_results(private_completion_context_st * const private_completion_context,
                                                       readline_st * const readline_ctx)
{

    if (private_completion_context->unique_match != NULL)
    {
        process_unique_match(private_completion_context, &readline_ctx->line_context);
    }
    else
    {
        process_multiple_matches(private_completion_context, readline_ctx);
    }

}

void do_word_completion(readline_st * const readline_ctx)
{
    if (readline_ctx->completion_callback != NULL)
    {
        line_context_st * const line_ctx = &readline_ctx->line_context;
        private_completion_context_st private_completion_context;

        if (!private_completion_context_init(&private_completion_context, line_ctx))
        {
            goto done;
        }

        readline_ctx->completion_callback(&private_completion_context.public_context,
                                          readline_ctx->user_completion_context);

        private_completion_context_process_results(&private_completion_context, readline_ctx);

        private_completion_context_teardown(&private_completion_context);
    }

done:
    return;
}

