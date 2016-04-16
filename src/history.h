#ifndef __HISTORY_H__
#define __HISTORY_H__

#include <stdbool.h>
#include <stddef.h>

typedef struct history_st history_st;

void history_free(history_st * const history);
history_st * history_alloc(size_t const max_entries);
bool history_add(history_st * const history, char const * const str);
void history_reset(history_st * const history);
bool history_currently_at_most_recent(history_st * const history);
char const * history_get_older_entry(history_st * const history);
char const * history_get_newer_entry(history_st * const history);

#endif /* __HISTORY_H__ */
