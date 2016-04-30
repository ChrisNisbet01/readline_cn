/* Copyright (C) Chris Nisbet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly 
 * prohibited. Proprietary and confidential. Written by Chris 
 * Nisbet <nisbet@ihug.co.nz>, April 2016.
 */

#ifndef __STRDUP_PARTIAL_H__
#define __STRDUP_PARTIAL_H__

#include <stddef.h>

char * strdup_partial(char const * const source, size_t const start, size_t const end);

#endif /* __STRDUP_PARTIAL_H__ */
