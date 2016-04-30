/* Copyright (C) Chris Nisbet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly 
 * prohibited. Proprietary and confidential. Written by Chris 
 * Nisbet <nisbet@ihug.co.nz>, April 2016.
 */

#ifndef __ARGS_H__
#define __ARGS_H__

#include <stddef.h>

typedef struct args_st args_st;
struct args_st
{
    size_t slots_allocated;
    size_t argc;
    char const * * argv;
}; 

void args_free(args_st const * const args);
args_st * args_alloc(unsigned int const initial_size);
int args_add_arg(args_st * const args, char const * const arg);

#endif /* __ARGS_H__ */
