#include "strdup_partial.h"

#include <string.h>
#include <stdlib.h>

char * strdup_partial(char const * const source, size_t const start, size_t const end)
{
    size_t bytes_to_copy;
    char * string;

    if (end < start)
    {
        string = NULL;
        goto done;
    }

    bytes_to_copy = end - start;

    if (source != NULL)
    {
        size_t const source_len = strlen(source);

        if (source_len < end)
        {
            string = NULL;
            goto done;
        }

        if (source_len < start)
        {
            string = NULL;
            goto done;
        }
    }

    string = malloc(sizeof(*string) * (bytes_to_copy + 1)); /* Include one extra byte for the NUL terminator */
    if (string == NULL)
    {
        goto done;
    }

    if (bytes_to_copy > 0)
    {
        memcpy(string, &source[start], bytes_to_copy);
    }
    string[bytes_to_copy] = '\0';

done:
    return string;
}


