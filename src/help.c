/* Copyright (C) Chris Nisbet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly 
 * prohibited. Proprietary and confidential. Written by Chris 
 * Nisbet <nisbet@ihug.co.nz>, April 2016.
 */

#include "help.h"
#include "tokenise.h"
#include "args.h"
#include "strdup_partial.h"
#include "common_prefix_length.h"
#include "print_words_in_columns.h"
#include "readline_context.h"
#include "terminal.h"
#include "utils.h"

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct private_help_context_st private_help_context_st;
struct private_help_context_st
{
    tokens_st * tokens;

    help_context_st public_context;
}; 

#define GET_PRIVATE_CONTEXT_FROM_PUBLIC_CONTEXT(help_context) \
    container_of(help_context, private_help_context_st, public_context)

static char const * get_token_at_index(help_context_st * const help_context, size_t const index)
{
    private_help_context_st * const private_help_context =
        GET_PRIVATE_CONTEXT_FROM_PUBLIC_CONTEXT(help_context); 

    return tokens_get_token_at_index(private_help_context->tokens, index);
}

static void private_help_context_teardown(private_help_context_st * const private_help_context)
{
    help_context_st * const help_context = &private_help_context->public_context;

    if (help_context->write_back_fd != -1)
    {
        close(help_context->write_back_fd);
    }

    tokens_free(private_help_context->tokens);
}

static bool private_help_context_init(private_help_context_st * const private_help_context, 
                                      line_context_st * const line_ctx,
                                      char const * const field_separators)
{
    help_context_st * const help_context = &private_help_context->public_context;
    bool init_ok;

    memset(private_help_context, 0, sizeof *private_help_context);

    private_help_context->tokens = tokenise_line(line_ctx->edit_buffer, 0, line_ctx->edit_index, true, field_separators);
    if (private_help_context->tokens == NULL)
    {
        init_ok = false;
        goto done;
    }

    /* The casts are required because the struct members are 
     * marked as const. 
     */
    *(int *)&help_context->write_back_fd = dup(line_ctx->terminal_fd);
    if (help_context->write_back_fd == -1)
    {
        init_ok = false;
        goto done;
    }
    *(size_t *)&help_context->current_token_index = tokens_get_current_token_index(private_help_context->tokens);
    *(char const * *)&help_context->current_token = tokens_get_current_token(private_help_context->tokens);
    *(size_t *)&help_context->num_tokens = tokens_get_num_tokens(private_help_context->tokens);
    *(help_tokens_get_token_at_index_fn *)&help_context->tokens_get_token_at_index_fn = get_token_at_index;

    init_ok = true;

done:
    if (!init_ok)
    {
        private_help_context_teardown(private_help_context);
    }
    return init_ok;
}

static void private_help_context_process_results(private_help_context_st * const private_help_context,
                                                       readline_st * const readline_ctx,
                                                       bool const characters_were_printed)
{
    bool need_to_redisplay_line;
    UNUSED_PARAMETER(private_help_context);

    need_to_redisplay_line = characters_were_printed;

    if (need_to_redisplay_line)
    {
        line_context_st * const line_ctx = &readline_ctx->line_context;

        redisplay_line(line_ctx);
    }
}

void do_help(readline_st * const readline_ctx)
{
    if (readline_ctx->help_callback != NULL)
    {
        int characters_were_printed;

        line_context_st * const line_ctx = &readline_ctx->line_context;
        private_help_context_st private_help_context;

        if (!private_help_context_init(&private_help_context, line_ctx, readline_ctx->field_separators))
        {
            goto done;
        }

        /* The idea is that the callback will write directly to the 
         * write_back_fd file descriptor anything that they like. The 
         * return value should be non-zero if anything was written, and 
         * in that case the current edit line will be redisplayed. 
         */
        characters_were_printed = readline_ctx->help_callback(&private_help_context.public_context,
                                                              readline_ctx->user_context);

        private_help_context_process_results(&private_help_context, readline_ctx, characters_were_printed > 0);

        private_help_context_teardown(&private_help_context);
    }

done:
    return;
}

