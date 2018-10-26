/* Copyright (C) Chris Nisbet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly 
 * prohibited. Proprietary and confidential. Written by Chris 
 * Nisbet <nisbet@ihug.co.nz>, April 2016.
 */

#include "terminal.h"

#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/select.h>

#define DEFAULT_SCREEN_COLUMNS 80

typedef struct terminal_settings_st terminal_settings_st;
struct terminal_settings_st
{
    struct termios settings;
};

int tty_put(int const out_fd, char const ch)
{
    return write(out_fd, &ch, sizeof ch);
}

void tty_puts(int const out_fd, char const * const string)
{
    char const * p = string;

    while (*p != '\0')
    {
        char const char_to_put = *p;

        tty_put(out_fd, char_to_put);
        p++;
    }
}

static bool wait_for_file_to_be_readable(int const fd, unsigned int const max_seconds_to_wait)
{
    int select_result;
    fd_set file_descriptor_set;
    struct timeval timeout;
    struct timeval * timeout_to_use;
    bool file_is_readable;

    if (max_seconds_to_wait > 0)
    {
        timeout.tv_sec = max_seconds_to_wait;
        timeout.tv_usec = 0;
        timeout_to_use = &timeout;
    }
    else
    {
        timeout_to_use = NULL; /* Block indefinitely. */
    }

    FD_ZERO(&file_descriptor_set);
    FD_SET(fd, &file_descriptor_set);

    do
    {
        select_result = select(fd + 1, &file_descriptor_set, NULL, NULL, timeout_to_use);
    }
    while (select_result == -1 && errno == EINTR);

    file_is_readable = select_result > 0;

    return file_is_readable;
}

static int read_char(int const in_fd, char * const ch)
{
    int r;

    do
    {
        r = read(in_fd, ch, sizeof *ch);
    } 
    while (r == -1 && errno == EINTR);

    return r;
}

tty_get_result_t tty_get(int const in_fd, unsigned int const maximum_seconds_to_wait, int * const character_read)
{
    int read_result;
    int r;
    char ch;

    if (maximum_seconds_to_wait > 0)
    {
        if (!wait_for_file_to_be_readable(in_fd, maximum_seconds_to_wait))
        {
            read_result = tty_get_result_timeout;
            goto done;
        }
    }

    r = read_char(in_fd, &ch);

    if (r != 1)
    {
        read_result = tty_get_result_eof;
        goto done;
    }

    if (character_read != NULL)
    {
        *character_read = ch;
    }

    read_result = tty_get_result_ok; 

done:
    return read_result;
}

static int getattr(int fd, struct termios * arg)
{
    int result;
    int tries = 3;

    do
    {
        result = tcgetattr(fd, arg);
        if (-1 == result)
        {
            if (EINTR != errno)
            {
                tries = 0;
            }
            else
            {
                tries--;
            }
        }
    }
    while (-1 == result && tries > 0);

    return result;
}

static int setattr(int fd, int opt, const struct termios * arg)
{
    int result;
    int tries = 3;

    do
    {
        result = tcsetattr(fd, opt, arg);
        if (-1 == result)
        {
            if (EINTR != errno)
            {
                tries = 0;
            }
            else
            {
                tries--;
            }
        }
    }
    while (-1 == result && tries > 0);

    return result;
}

terminal_settings_st * terminal_prepare(void)
{
    terminal_settings_st * previous_terminal_settings;
    struct termios new_terminal_settings;

    previous_terminal_settings = malloc(sizeof *previous_terminal_settings);
    if (previous_terminal_settings == NULL)
    {
        goto done;
    }

    if (-1 == getattr(0, &previous_terminal_settings->settings))
    {
        perror("Failed tcgetattr()");
    }

    /* Base the new settings off the original settings. */
    new_terminal_settings = previous_terminal_settings->settings;
    /* Make the required changes. */
    new_terminal_settings.c_lflag &= ~(ECHO | ICANON | ISIG); /* no echo, canonical mode off, no signals */
    new_terminal_settings.c_iflag &= ~(INPCK | ISTRIP); /* turn off parity check, don't strip top bit */

    new_terminal_settings.c_cc[VMIN] = 1; /* one char minimum */
    new_terminal_settings.c_cc[VTIME] = 0; /* no timeout */

    if (-1 == setattr(0, TCSADRAIN, &new_terminal_settings))
    {
        perror("Failed tcsetattr(TCSADRAIN)");
    }

done:
    return previous_terminal_settings;
}

void terminal_restore(terminal_settings_st * const previous_terminal_settings)
{
    if (previous_terminal_settings != NULL)
    {
        if (-1 == setattr(0, TCSADRAIN, &previous_terminal_settings->settings))
        {
            perror("Failed tcsetattr(TCSADRAIN)");
        }
        free(previous_terminal_settings);
    }
}

size_t terminal_get_width(int const out_fd)
{
    struct winsize window;
    int width;

    if (ioctl(out_fd, TIOCGWINSZ, &window) >= 0 && window.ws_col > 0 && window.ws_row > 0)
    {
        width = (size_t)window.ws_col;
    }
    else
    {
        width = DEFAULT_SCREEN_COLUMNS;
    }

    return width;
}

static bool move_physical_cursor(int const out_fd, size_t const amount_to_move, char const direction)
{
    bool cursor_moved;

    if (amount_to_move > 0)
    {
        char buffer[20];

        snprintf(buffer, sizeof buffer, "\033[%zu%c", amount_to_move, direction);

        // TODO: check for write error
        tty_puts(out_fd, buffer);
    }
    cursor_moved = true;

    return cursor_moved;
}

bool terminal_move_physical_cursor_up(int const out_fd, size_t const rows)
{
    return move_physical_cursor(out_fd, rows, 'A');
}

bool terminal_move_physical_cursor_down(int const out_fd, size_t const rows)
{
    return move_physical_cursor(out_fd, rows, 'B');
}

bool terminal_move_physical_cursor_right(int const out_fd, size_t const columns)
{
    return move_physical_cursor(out_fd, columns, 'C');
}

bool terminal_move_physical_cursor_left(int const out_fd, size_t const columns)
{
    return move_physical_cursor(out_fd, columns, 'D');
}

void terminal_delete_to_end_of_line(int const out_fd)
{
    tty_puts(out_fd, "\033[K");
}
