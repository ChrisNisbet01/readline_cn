#include "split_path.h"
#include "strdup_partial.h"

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/* Split a pathname into allocated directory and trailing filename parts. */
int split_path(char const * const path, char const * * const dir_part, char const * * const file_part, size_t * const file_part_start)
{
    /* Not using dirname() and basename() because they don't 
     * return strings suitable for using with opendir(). 
     * e.g. with input string "/abc/" we want 
     * dir: "/abc/" and file "" 
     * but with dirname/basename we get 
     * dir "/" and file "abc" 
     */
    static char dot[] = ".";
    char * dpart = NULL;
    char * fpart = NULL;
    char * last_slash;
    bool path_is_split;
    size_t start_of_fpart;

    last_slash = strrchr(path, '/');
    if (last_slash == NULL)
    {
        /* no mention of a path prefix. */
        if ((dpart = strdup(dot)) == NULL)
        {
            path_is_split = false;
            goto done;
        }
        if ((fpart = strdup(path)) == NULL)
        {
            path_is_split = false;
            goto done;
        }
        start_of_fpart = 0;
    }
    else
    {
        char const * const char_after_last_slash = last_slash + 1;

        /* include the trailing slash in the directory part */
        dpart = strdup_partial(path, 0, char_after_last_slash - path);
        if (dpart == NULL)
        {
            path_is_split = false;
            goto done;
        }

        fpart = strdup(char_after_last_slash);
        if (fpart == NULL)
        {
            path_is_split = false;
            goto done;
        }
        start_of_fpart = char_after_last_slash - path; 
    }
    path_is_split = true;

done:
    if (path_is_split)
    {
        *dir_part = dpart;
        *file_part = fpart;
        if (file_part_start != NULL)
        {
            *file_part_start = start_of_fpart;
        }
    }
    else
    {
        free(dpart);
        free(fpart);
    }

    return path_is_split;
}


