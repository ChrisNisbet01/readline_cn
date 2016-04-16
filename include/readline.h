#ifndef __READLINE_H__
#define __READLINE_H__

#include <stdio.h>
#include <stddef.h>

typedef struct editline_st editline_st;

typedef enum editline_result_t editline_result_t;
enum editline_result_t
{
    editline_result_success,
    editline_result_ctrl_c,
    editline_result_timed_out,
    editline_result_eof,
    editline_result_error   /* General error */
};

typedef struct completion_context_st completion_context_st;

typedef int (* completion_possible_word_add_fn)(completion_context_st * const completion_context,
                                                char const * const possible_word);
typedef void (* completion_start_index_set_fn)(completion_context_st * const completion_context,
                                                size_t const completion_start_index);
typedef int (* completion_unique_match_set_fn)(completion_context_st * const completion_context,
                                                char const * const unique_match);
typedef int (* completion_callback_fn)(completion_context_st * const completion_context,
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

    FILE * write_back_fp;
};

editline_st * editline_context_create(void * const user_completion_context,
                                      completion_callback_fn const completion_callback,
                                      int input_fd,
                                      int output_fd,
                                      size_t const history_size);
void editline_context_destroy(editline_st * const editline_ctx);

editline_result_t readline(editline_st * const editline_ctx,
                           unsigned int const timeout_seconds,
                           char const * const prompt,
                           char * * const line);

editline_result_t readline_args(editline_st * const editline_ctx,
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
