/* Copyright (C) Chris Nisbet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly 
 * prohibited. Proprietary and confidential. Written by Chris 
 * Nisbet <nisbet@ihug.co.nz>, April 2016.
 */

#ifndef __READLINE_H__
#define __READLINE_H__

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct readline_st readline_st;

typedef enum readline_result_t
{
    readline_result_success,
    readline_result_ctrl_c,
    readline_result_timed_out,
    readline_result_eof,
    readline_result_error   /* General error */
} readline_result_t;

typedef struct completion_context_st completion_context_st;
typedef struct help_context_st help_context_st; 

typedef int (* completion_possible_word_add_fn)(completion_context_st * const completion_context,
                                                char const * const possible_word);
typedef void (* completion_start_index_set_fn)(completion_context_st * const completion_context,
                                                size_t const completion_start_index);
typedef int (* completion_unique_match_set_fn)(completion_context_st * const completion_context,
                                                char const * const unique_match);
typedef int (* completion_callback_fn)(completion_context_st * const completion_context,
                                       void * const user_context);
typedef int (* help_callback_fn)(help_context_st * const completion_context,
                                       void * const user_context); 

typedef size_t (* tokens_get_current_token_index_fn)(completion_context_st * const completion_context);
typedef char const * (* tokens_get_current_token_fn)(completion_context_st * const completion_context);
typedef size_t (* tokens_get_num_tokens_fn)(completion_context_st * const completion_context);
typedef char const * (* tokens_get_token_at_index_fn)(completion_context_st * const completion_context, size_t const index); 


struct completion_context_st
{
    completion_possible_word_add_fn possible_word_add_fn;
    completion_start_index_set_fn start_index_set_fn;
    completion_unique_match_set_fn unique_match_set_fn;

    tokens_get_current_token_index_fn tokens_get_current_token_index_fn;
    tokens_get_current_token_fn tokens_get_current_token_fn;
    tokens_get_num_tokens_fn tokens_get_num_tokens_fn;
    tokens_get_token_at_index_fn tokens_get_token_at_index_fn;

    int write_back_fd;
};

typedef size_t (* help_tokens_get_current_token_index_fn)(help_context_st * const help_context);
typedef char const * (* help_tokens_get_current_token_fn)(help_context_st * const help_context);
typedef size_t (* help_tokens_get_num_tokens_fn)(help_context_st * const help_context);
typedef char const * (* help_tokens_get_token_at_index_fn)(help_context_st * const help_context, size_t const index);

struct help_context_st
{
    size_t const current_token_index;
    char const * const current_token;
    size_t const num_tokens;
    help_tokens_get_token_at_index_fn const tokens_get_token_at_index_fn; 

    int const write_back_fd;
}; 

readline_st * readline_context_create(void * const user_context,
                                      completion_callback_fn const completion_callback,
                                      help_callback_fn const help_callback,
                                      char const help_key,
                                      int input_fd,
                                      int output_fd,
                                      size_t const history_size);
void readline_context_destroy(readline_st * const readline_ctx);

bool readline_history_control(readline_st * const readline_ctx, bool const enable);
char readline_set_mask_character(readline_st * const readline_ctx, char const mask_character);
void readline_set_field_separators(readline_st * const readline_ctx, char const * const field_separators);
size_t readline_set_maximum_line_length(readline_st * const readline_ctx, size_t const maximum_line_length); 
void readline_set_initial_timeout_check(readline_st * const readline_ctx, bool const do_initial_check);

readline_result_t readline(readline_st * const readline_ctx,
                           unsigned int const timeout_seconds,
                           char const * const prompt,
                           char * * const line);

readline_result_t readline_args(readline_st * const readline_ctx,
                                unsigned int const timeout_seconds,
                                char const * const prompt,
                                size_t * const argc,
                                char const * * * argv);

/* Standard filename completion callback. User callbacks can 
 * call this function from their own completion callback if they
 * want filename completion. 
 */
int do_filename_completion(completion_context_st * const completion_context,
                           void * const user_context);


#endif  /* __READLINE_H__ */
