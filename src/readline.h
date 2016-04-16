#ifndef __READLINE_PRIVATE_H__
#define __READLINE_PRIVATE_H__

#include "../include/readline.h"
#include "args.h"
#include "line_context.h"
#include "tokenise.h"

#include <termios.h>
#include <stdbool.h>

#define CTL(x)          ((x) & 0x1F)
#define ISCTL(x)        ((x) && (x) < 0x20)

typedef struct private_completion_context_st private_completion_context_st;
struct private_completion_context_st
{
    args_st * possible_words;
    char const * unique_match;  /* if non-NULL, will override any possible words */
    char * freeform_text;
    size_t freeform_text_len;
    size_t completion_start_index;
    tokens_st * tokens; 

    completion_context_st public_context;
}; 

#endif /* __READLINE_PRIVATE_H__ */
