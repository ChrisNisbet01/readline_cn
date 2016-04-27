#ifndef __SCREEN_H__
#define __SCREEN_H__

#include "line_context.h"

void screen_put(line_context_st * const line_ctx, char const ch);
void screen_puts(line_context_st * const line_ctx, char const * const string, char const mask_character);

#endif /* __SCREEN_H__ */
