#ifndef __HANDLERS_H__
#define __HANDLERS_H__

#include "readline_status.h"
#include "readline_context.h"

readline_status_t handle_control_char(readline_st * const readline_ctx, int const ch);
readline_status_t handle_escaped_char(readline_st * const readline_ctx);
void handle_regular_char(readline_st * const readline_ctx, int const ch, bool const update_terminal);
void handle_backspace(readline_st * const readline_ctx);

#endif /* __HANDLERS_H__ */
