/* Copyright (C) Chris Nisbet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly 
 * prohibited. Proprietary and confidential. Written by Chris 
 * Nisbet <nisbet@ihug.co.nz>, April 2016.
 */

#include "directory.h"
#include "directory_test.h"

#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

static bool full_path_is_a_directory(char const * const full_path)
{
    bool is_directory;
    struct stat file_stat;

    is_directory = STAT(full_path, &file_stat) == 0 && S_ISDIR(file_stat.st_mode);

    return is_directory;
}

int check_if_path_is_a_directory(char const * const directory, char const * const filename, bool * const is_a_directory)
{
    int result;
    char * full_path = NULL;

    if (asprintf(&full_path, "%s/%s", directory, filename) < 0)
    {
        result = -1;
        goto done;
    }
    *is_a_directory = full_path_is_a_directory(full_path);
    result = 0;

done:
    free(full_path);

    return result;
}


