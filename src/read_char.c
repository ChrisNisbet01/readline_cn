#include "read_char.h"
#include "terminal.h"

int read_char_from_input_descriptor(int const input_fd, unsigned int const maximum_seconds_to_wait, readline_status_t * const readline_status)
{
    int ch;
    readline_status_t status;
    tty_get_result_t tty_get_result;

    tty_get_result = tty_get(input_fd, maximum_seconds_to_wait, &ch);
    switch (tty_get_result)
    {
        case tty_get_result_eof:
            status = readline_status_eof;
            goto done;

        case tty_get_result_timeout:
            status = readline_status_timed_out;
            goto done;
        case tty_get_result_ok:
            break;
    }
    status = readline_status_continue;

done:
    *readline_status = status;
    return ch;
}


