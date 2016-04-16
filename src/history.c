#include "history.h"
#include "history_entries.h"

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#define HISTORY_IS_VALID(history) ((history) != NULL)

struct history_st
{
    size_t max_entries;
    history_entries_st * entries;
    history_entry_st const * current_entry;
}; 

static void history_remove_and_free_entry(history_st * const history, history_entry_st * const entry)
{
    history_remove_entry_from_list(history->entries, entry);
    history_entry_free(entry);
}

static void history_add_entry(history_st * const history, history_entry_st * const entry)
{
    history_add_new_entry_to_list(history->entries, entry);
}

static bool string_is_all_whitespace(char const * const string)
{
    bool is_all_whitespace;
    char const * pch;

    for (pch = string; *pch != '\0'; pch++)
    {
        if(!isspace((int)*pch))
        {
            is_all_whitespace = false;
            goto done;
        }
    }

    is_all_whitespace = true;

done:
    return is_all_whitespace;
}

static bool string_is_duplicate_of_most_recent(history_st * const history, char const * const str)
{
    bool is_duplicate_of_last;
    history_entry_st const * entry;

    entry = history_get_newest_entry_from_list(history->entries);
    if (entry == NULL)
    {
        /* No entries at all. */
        is_duplicate_of_last = false;
        goto done;
    }
    is_duplicate_of_last = strcmp(history_entry_get_value(entry), str) == 0;

done:
    return is_duplicate_of_last;
}

static void history_remove_oldest_entry(history_st * const history)
{
    history_entry_st * const oldest_entry = history_get_oldest_entry_from_list(history->entries);

    if (oldest_entry != NULL)
    {
        history_remove_and_free_entry(history, oldest_entry);
    }
}


static history_entry_st * history_get_previous_entry(history_st * const history, history_entry_st const * const entry)
{
    history_entry_st * previous_entry;

    if (entry == NULL)
    {
        previous_entry = history_get_newest_entry_from_list(history->entries);
        /* May still be NULL if there are no entries. */
    }
    else
    {
        previous_entry = history_get_older_entry_from_list(history->entries, history->current_entry);
        /* Shouldn't be NULL as we've already checked that we weren't 
         * at the first entry. 
         */
    }

    return previous_entry;
}

static history_entry_st * history_get_next_entry(history_st * const history, history_entry_st const * const entry)
{
    history_entry_st * next_entry;

    if (entry != NULL)
    {
        next_entry = history_get_newer_entry_from_list(history->entries, entry);
    }
    else
    {
        next_entry = NULL;
    }

    return next_entry;
}

static bool current_entry_is_oldest(history_st * const history, history_entry_st const * const entry)
{
    return entry == history_get_oldest_entry_from_list(history->entries);
}

void history_free(history_st * const history)
{
    if (HISTORY_IS_VALID(history))
    {
        history_entries_free(history->entries);
        free(history);
    }
}

history_st * history_alloc(size_t const max_entries)
{
    history_st * history;

    /* Note that specifying a max_entries value of 0 will result 
     * in unlimited history. 
     */
    history = calloc(1, sizeof *history);
    if (history == NULL)
    {
        goto done;
    }
    history->max_entries = max_entries;
    history->entries = history_entries_alloc();
    if (history->entries == NULL)
    {
        free(history);
        history = NULL;
        goto done;
    }

done:
    return history;
}

bool history_add(history_st * const history, char const * const str)
{
    bool added;
    history_entry_st * new_entry;

    if (!HISTORY_IS_VALID(history))
    {
        added = false;
        goto done;
    }

    if (string_is_all_whitespace(str))
    {
        added = false;
        goto done;
    }

    if (string_is_duplicate_of_most_recent(history, str))
    {
        added = false;
        goto done;
    }

    new_entry = history_entry_alloc(str);
    if (new_entry == NULL)
    {
        added = false;
        goto done;
    }

    if (history_entries_get_count(history->entries) == history->max_entries)
    {
        /* History is full. Remove the oldest entry. */
        history_remove_oldest_entry(history);
    }

    history_add_entry(history, new_entry);

    added = true;

done:
    return added;
}

void history_reset(history_st * const history)
{
    if (HISTORY_IS_VALID(history))
    {
        history->current_entry = NULL;
    }
}

bool history_currently_at_most_recent(history_st * const history)
{
    bool at_most_recent;

    if (HISTORY_IS_VALID(history))
    {
        at_most_recent = history->current_entry == NULL;
    }
    else
    {
        at_most_recent = false;
    }

    return at_most_recent;
}

char const * history_get_older_entry(history_st * const history)
{
    char const * line;

    if (!HISTORY_IS_VALID(history))
    {
        line = NULL;
        goto done;
    }

    if (current_entry_is_oldest(history, history->current_entry))
    {
        line = NULL;
        goto done;
    }

    history->current_entry = history_get_previous_entry(history, history->current_entry);

    if (history->current_entry == NULL)
    {
        line = NULL;
        goto done;
    }
    line = history_entry_get_value(history->current_entry);

done:
    return line;
}

char const * history_get_newer_entry(history_st * const history)
{
    char const * line;

    if (!HISTORY_IS_VALID(history))
    {
        line = NULL;
        goto done;
    }

    history->current_entry = history_get_next_entry(history, history->current_entry);

    if (history->current_entry == NULL)
    {
        line = NULL;
        goto done;
    }
    line = history_entry_get_value(history->current_entry);

done:
    return line;
}
