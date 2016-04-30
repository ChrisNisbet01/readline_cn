/* Copyright (C) Chris Nisbet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly 
 * prohibited. Proprietary and confidential. Written by Chris 
 * Nisbet <nisbet@ihug.co.nz>, April 2016.
 */

#ifndef __DIRECTORY_TEST_H__
#define __DIRECTORY_TEST_H__

#include <sys/stat.h>

#if defined(_UNIT_TEST)
#define STAT test_stat
#else
#define STAT stat
#endif

int test_stat(const char * file, struct stat * buf); 

#endif /* __DIRECTORY_TEST_H__ */
