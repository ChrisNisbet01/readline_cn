/* Copyright (C) Chris Nisbet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly 
 * prohibited. Proprietary and confidential. Written by Chris 
 * Nisbet <nisbet@ihug.co.nz>, April 2016.
 */

#ifndef __READ_CHAR_H__
#define __READ_CHAR_H__

#include "readline_status.h"

int read_char_from_input(int const input_fd,
                         unsigned int const maximum_seconds_to_wait,
                         readline_status_t * const readline_status);

#endif /* __READ_CHAR_H__ */
