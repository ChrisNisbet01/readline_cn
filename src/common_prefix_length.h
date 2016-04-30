/* Copyright (C) Chris Nisbet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly 
 * prohibited. Proprietary and confidential. Written by Chris 
 * Nisbet <nisbet@ihug.co.nz>, April 2016.
 */

#ifndef __COMMON_PREFIX_LENGTH_H__
#define __COMMON_PREFIX_LENGTH_H__

#include <stddef.h>

size_t find_common_prefix_length(size_t const num_words, char const * const * const words);

#endif /* __COMMON_PREFIX_LENGTH_H__ */
