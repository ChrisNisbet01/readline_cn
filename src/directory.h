/* Copyright (C) Chris Nisbet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly 
 * prohibited. Proprietary and confidential. Written by Chris 
 * Nisbet <nisbet@ihug.co.nz>, April 2016.
 */

#ifndef __DIRECTORY_H__
#define __DIRECTORY_H__

#include <stdbool.h>

int check_if_path_is_a_directory(char const * const directory, char const * const filename, bool * const is_a_directory);

#endif /* __DIRECTORY_H__ */
