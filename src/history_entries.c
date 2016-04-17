#include "history_entries.h"

#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>

typedef struct history_entries_list_st history_entries_list_st;
TAILQ_HEAD(history_entries_list_st, history_entry_st);

struct history_entries_st
{
    history_entries_list_st list;
    size_t num_entries;
};

struct history_entry_st
{
    TAILQ_ENTRY(history_entry_st) entry;
    char const * value;
}; 

static void history_init_list(history_entries_st * const entries)
{
    TAILQ_INIT(&entries->list);
}

static void history_entries_init(history_entries_st * const entries)
{
    history_init_list(entries);
    entries->num_entries = 0;
}

static void history_free_all_entries(history_entries_st * const entries)
{
    history_entry_st * entry;

    for (entry = history_get_oldest_entry_from_list(entries);
         entry != NULL;
         entry = history_get_oldest_entry_from_list(entries))
    {
        history_remove_entry_from_list(entries, entry);
        history_entry_free(entry);
    }
}

history_entry_st * history_get_oldest_entry_from_list(history_entries_st const * const entries)
{
    return TAILQ_FIRST(&entries->list);
}

history_entry_st * history_get_newest_entry_from_list(history_entries_st const * const entries)
{
    return TAILQ_LAST(&entries->list, history_entries_list_st);
}

history_entry_st * history_get_older_entry_from_list(history_entries_st const * const entries, history_entry_st const * const entry)
{
    /* Including 'entries' in the prototype to provide a consistent 
     * API, where the list head is always passed to functions that 
     * may get/set the list in some way. 
    */
    history_entry_st * older_entry;

    (void)entries;
    if (entry == NULL)
    {
        older_entry = NULL;
    }
    else
    {
        older_entry = TAILQ_PREV(entry, history_entries_list_st, entry);
    }

    return older_entry;
}

history_entry_st * history_get_newer_entry_from_list(history_entries_st const * const entries, history_entry_st const * const entry)
{
    /* Including 'entries' in the prototype to provice a consistent 
     * API, where the list head is always passed to functions that 
     * may get/set the list in some way. 
    */
    (void)entries;
    history_entry_st * newer_entry;

    (void)entries;
    if (entry == NULL)
    {
        newer_entry = NULL;
    }
    else
    {
        newer_entry = TAILQ_NEXT(entry, entry); 
    }

    return newer_entry;
}

void history_remove_entry_from_list(history_entries_st * const entries, history_entry_st * const entry)
{
    TAILQ_REMOVE(&entries->list, entry, entry);
    entries->num_entries--;
}

void history_add_new_entry_to_list(history_entries_st * const entries, history_entry_st * const entry)
{
    TAILQ_INSERT_TAIL(&entries->list, entry, entry);
    entries->num_entries++;
}

void history_entry_free(history_entry_st * const entry)
{
    if (entry != NULL)
    {
        free((void *)entry->value);
        free(entry);
    }
}

history_entry_st * history_entry_alloc(char const * const value)
{
    history_entry_st * entry;

    entry = calloc(1, sizeof *entry);
    if (entry == NULL)
    {
        goto done;
    }

    if (value != NULL)
    {
        entry->value = strdup(value);
        if (entry->value == NULL)
        {
            history_entry_free(entry);
            entry = NULL;
            goto done;
        }
    }

done:
    return entry;
}

char const * history_entry_get_value(history_entry_st const * const entry)
{
    return entry->value;
}

size_t history_entries_get_count(history_entries_st * const entries)
{
    return entries->num_entries;
}

history_entries_st * history_entries_alloc(void)
{
    history_entries_st * const entries = calloc(1, sizeof(history_entries_st));

    if (entries == NULL)
    {
        goto done;
    }
    history_entries_init(entries);

done:
    return entries;
}

void history_entries_free(history_entries_st * const entries)
{
    if (entries != NULL)
    {
        history_free_all_entries(entries);
        free(entries);
    }
}

