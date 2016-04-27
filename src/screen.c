#include "screen.h"
#include "terminal.h"

#define MAX(x,y) ((x) > (y) ? (x) : (y))

void screen_put(line_context_st * const line_ctx, char const ch)
{
    tty_put(line_ctx->terminal_fd, ch);
    line_ctx->screen_cursor_index++;
    if (line_ctx->screen_cursor_index == line_ctx->terminal_width)
    {
        tty_put(line_ctx->terminal_fd, '\n');
        line_ctx->screen_cursor_index = 0;
        line_ctx->screen_cursor_row++;
        line_ctx->num_rows = MAX(line_ctx->num_rows, line_ctx->screen_cursor_row + 1);
    }
}

void screen_puts(line_context_st * const line_ctx, char const * const string, char const mask_character)
{
    char const * p = string;

    while ( *p != '\0')
    {
        char const char_to_put = (mask_character != '\0') ? mask_character : *p;

        screen_put(line_ctx, char_to_put);
        p++;
    }
}


