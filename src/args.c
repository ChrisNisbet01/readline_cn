#include "args.h"

#include <stdlib.h>
#include <string.h>

#define INCREMENT	1

static void free_args_array(size_t const num_args, char const * * const args)
{
    size_t index;

    for (index = 0; index < num_args; index++)
    {
        free((void *)args[index]);
    }
    free(args);
}

void args_free(args_st const * const args)
{
    if (args != NULL)
    {
        if (args->argv != NULL)
        {
            free_args_array(args->argc, args->argv);
        }
        free((void *)args);
    }
}

args_st * args_alloc(unsigned int const initial_size)
{
    args_st * args;

    args = calloc(1, sizeof *args);
    if (args == NULL)
    {
        goto error;
    }
    if (initial_size > 0)
    {
        args->slots_allocated = initial_size;
        args->argv = calloc(initial_size, sizeof(*args->argv));
        if (args->argv == NULL)
        {
            goto error;
        }
    }

    goto done;

error:
    args_free(args);
    args = NULL;
done:
    return args;
}

int args_add_arg(args_st * const args, char const * const arg)
{
    int arg_added;

    if (args->argc == args->slots_allocated)
    {
        args->argv = realloc(args->argv, (args->slots_allocated + INCREMENT) * sizeof(*args->argv));
        if (args->argv == NULL)
        {
            args->argc = 0;
            arg_added = 0;
            goto done;
        }
        args->slots_allocated += INCREMENT;
    }
    if (arg != NULL)
    {
        args->argv[args->argc] = strdup(arg);
        if (args->argv[args->argc] == NULL)
        {
            arg_added = 0;
            goto done;
        }
        args->argc++;
    }
    else
    {
        args->argv[args->argc] = NULL;
    }
    arg_added = 1;

done:
    return arg_added;
}


