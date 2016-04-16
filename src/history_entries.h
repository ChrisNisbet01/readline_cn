#ifndef __HISTORY_ENTRIES_H__
#define __HISTORY_ENTRIES_H__

#include <stddef.h>

typedef struct history_entry_st history_entry_st;
typedef struct history_entries_st history_entries_st;

history_entry_st * history_get_oldest_entry_from_list(history_entries_st const * const entries);
history_entry_st * history_get_newest_entry_from_list(history_entries_st const * const entries);
history_entry_st * history_get_older_entry_from_list(history_entries_st const * const entries, history_entry_st const * const entry);
history_entry_st * history_get_newer_entry_from_list(history_entries_st const * const entries, history_entry_st const * const entry);
void history_remove_entry_from_list(history_entries_st * const entries, history_entry_st * const entry);
void history_add_new_entry_to_list(history_entries_st * const entries, history_entry_st * const entry);
void history_entry_free(history_entry_st * const entry);
history_entry_st * history_entry_alloc(char const * const value);
char const * history_entry_get_value(history_entry_st const * const entry);
size_t history_entries_get_count(history_entries_st * const entries);
history_entries_st * history_entries_alloc(void);
void history_entries_free(history_entries_st * const entries);

#endif /* __HISTORY_ENTRIES_H__ */
