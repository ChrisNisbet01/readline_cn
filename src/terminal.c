#include "terminal.h"

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/select.h>

#define DEFAULT_SCREEN_COLUMNS 80

void tty_put(int const out_fd, char const ch)
{
    // TODO: Don't ignore return value.
    (void)write(out_fd, &ch, sizeof ch);
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
    fd_set fd_set;
    struct timeval timeout;
    bool file_is_readable;

    timeout.tv_sec = max_seconds_to_wait;
    timeout.tv_usec = 0;

    FD_ZERO(&fd_set);
    FD_SET(fd, &fd_set);

    do
    {
        select_result = select(fd + 1, &fd_set, NULL, NULL, &timeout);
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
    }while (-1 == result && tries > 0);

    return result;
}

void terminal_prepare(struct termios * const previous_terminal_settings)
{
    struct termios new_terminal_settings;

    if (-1 == getattr(0, previous_terminal_settings))
    {
        perror("Failed tcgetattr()");
    }

    new_terminal_settings = *previous_terminal_settings;
    new_terminal_settings.c_lflag &= ~(ECHO | ICANON | ISIG); /* no echo, canonical mode off, no signals */
    new_terminal_settings.c_iflag &= ~(INPCK | ISTRIP); /* turn off parity check, don't strip top bit */

    new_terminal_settings.c_cc[VMIN] = 1; /* one char minimum */
    new_terminal_settings.c_cc[VTIME] = 0; /* no timeout */

    if (-1 == setattr(0, TCSADRAIN, &new_terminal_settings))
    {
        perror("Failed tcsetattr(TCSADRAIN)");
    }
}

void terminal_restore(struct termios * const previous_terminal_settings)
{
    if (-1 == setattr(0, TCSADRAIN, previous_terminal_settings))
    {
        perror("Failed tcsetattr(TCSADRAIN)");
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

bool terminal_move_physical_cursor_right(int const out_fd, size_t const columns)
{
    return move_physical_cursor(out_fd, columns, 'C');
}

bool terminal_move_physical_cursor_left(int const out_fd, size_t const columns)
{
    return move_physical_cursor(out_fd, columns, 'D');
}

bool terminal_move_physical_cursor_up(int const out_fd, size_t const rows)
{
    return move_physical_cursor(out_fd, rows, 'A');
}

bool terminal_move_physical_cursor_down(int const out_fd, size_t const rows)
{
    return move_physical_cursor(out_fd, rows, 'B');
}

void terminal_delete_to_end_of_line(int const out_fd)
{
    tty_puts(out_fd, "\033[K");
}
