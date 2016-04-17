#include "readline_context.h"

#include <stdlib.h>
#include <unistd.h>

static readline_st * readline_context_alloc(void)
{
    readline_st * readline_ctx;

    readline_ctx = (readline_st *)calloc(1, sizeof *readline_ctx);

    return readline_ctx;
}

static void readline_context_free(readline_st * const readline_ctx)
{
    line_buffer_teardown(&readline_ctx->line_context);
    history_free(readline_ctx->history);
    free((void *)readline_ctx->saved_line);
    free(readline_ctx);
}

readline_st * readline_context_create(void * const user_completion_context, 
                                      completion_callback_fn const completion_callback,
                                      int input_fd,
                                      int output_fd,
                                      size_t const history_size)
{
    readline_st * readline_ctx;

    readline_ctx = readline_context_alloc();
    if (readline_ctx == NULL)
    {
        goto done;
    }
    readline_ctx->insert_mode = true;

    readline_ctx->user_completion_context = user_completion_context;
    readline_ctx->completion_callback = completion_callback;
    readline_ctx->out_fd = output_fd;
    readline_ctx->in_fd = input_fd;
    readline_ctx->is_a_terminal = isatty(readline_ctx->in_fd);
    readline_ctx->history = history_alloc(history_size);
    readline_ctx->history_enabled = true;

done:

    return readline_ctx;
}

void readline_context_destroy(readline_st * const readline_ctx)
{
    readline_context_free(readline_ctx);
}

bool readline_context_history_control(readline_st * const readline_ctx, bool const enable)
{
    bool const previous_enable_state = readline_ctx->history_enabled;

    readline_ctx->history_enabled = enable;

    return previous_enable_state;
}
