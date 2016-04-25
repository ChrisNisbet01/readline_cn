#ifndef __READ_CHAR_H__
#define __READ_CHAR_H__

#include "readline_status.h"

int read_char_from_input_descriptor(int const input_fd,
                                    unsigned int const maximum_seconds_to_wait,
                                    readline_status_t * const readline_status);

#endif /* __READ_CHAR_H__ */
