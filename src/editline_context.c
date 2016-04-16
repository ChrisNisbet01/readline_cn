#include "editline_context.h"

#include <stdlib.h>
#include <unistd.h>

static editline_st * editline_context_alloc(void)
{
    editline_st * editline_ctx;

    editline_ctx = (editline_st *)calloc(1, sizeof *editline_ctx);

    return editline_ctx;
}

static void editline_context_free(editline_st * const editline_ctx)
{
    line_buffer_teardown(&editline_ctx->line_context);
    history_free(editline_ctx->history);
    free((void *)editline_ctx->saved_line);
    free(editline_ctx);
}

editline_st * editline_context_create(void * const user_completion_context, 
                                      completion_callback_fn const completion_callback,
                                      int input_fd,
                                      int output_fd,
                                      size_t const history_size)
{
    editline_st * editline_ctx;

    editline_ctx = editline_context_alloc();
    if (editline_ctx == NULL)
    {
        goto done;
    }
    editline_ctx->insert_mode = true;

    editline_ctx->user_completion_context = user_completion_context;
    editline_ctx->completion_callback = completion_callback;
    editline_ctx->history = history_alloc(history_size);
    editline_ctx->out_fd = output_fd;
    editline_ctx->in_fd = input_fd;
    editline_ctx->is_a_terminal = isatty(editline_ctx->in_fd);

done:

    return editline_ctx;
}

void editline_context_destroy(editline_st * const editline_ctx)
{
    editline_context_free(editline_ctx);
}


