/*
 * Simple hash table implementation.
 *
 * Copyright (C) Matthew Turner 2009. All rights reserved.
 *
 * $Id$
 */

#include "common.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "hash.h"


typedef struct _hash_table_entry_t hash_table_entry_t;

struct _hash_table_t
{
    unsigned size;
    unsigned count;
    double max_load;
    double min_load;
    hash_table_entry_t **entries;
};

struct _hash_table_entry_t
{
    const char *key;
    void *data;
    struct _hash_table_entry_t *next;
};

struct _hash_table_iterator_t
{
    unsigned slot;
    hash_table_entry_t *entry;
    hash_table_t *tbl;
    int end;
};

typedef uint32_t hash_t;


static hash_t (*hash) (const char *str);

static void hash_table_resize (hash_table_t *tbl, unsigned new_size);
static void hash_table_iterator_find_first (hash_table_iterator_t *iter);


/* hash table - public functions */

/* Make a new hash table, initially of size size */
hash_table_t *hash_table_new (unsigned size, double max_load, double min_load)
{
    hash_table_t *tbl = (hash_table_t *)calloc(sizeof(hash_table_t), 1);


    trce("hash_table_new(%u)\n", size);

    tbl->size = size;
    tbl->max_load = max_load;
    tbl->min_load = min_load;
    tbl->entries = (hash_table_entry_t **)calloc(sizeof(hash_table_entry_t *) * tbl->size, 1);


    return tbl;
}

void hash_table_delete (hash_table_t *tbl)
{
    assert(!tbl->count);
    free(tbl->entries);
    free(tbl);
}

static void add_to_entries (hash_table_entry_t **entries, unsigned entries_len, const char *key, void *data)
{
    hash_t h = hash(key);
    hash_table_entry_t  *e  = (hash_table_entry_t *)malloc(sizeof(hash_table_entry_t)),
                       **oe = &(entries[h % entries_len]);


    e->key  = key;
    e->data = data;
    e->next = *oe;

    *oe = e;
}

void hash_table_add (hash_table_t *tbl, const char *key, void *data)
{
    add_to_entries(tbl->entries, tbl->size, key, data);

    ++tbl->count;


    /* If the table's load average is above the configured maximum threshold,
     * double its size */
    if (hash_table_get_load_factor(tbl) > tbl->max_load)
    {
        trce("hash table load factor too great, doubling size to %u\n", tbl->size * 2);
        hash_table_resize(tbl, tbl->size * 2);
    }
}

void *hash_table_find (hash_table_t *tbl, const char *key)
{
    hash_t h = hash(key);
    hash_table_entry_t *e = tbl->entries[h % tbl->size];


    while (e)
    {
        if (!strcmp(e->key, key))
        {
            return e->data;
        }

        e = e->next;
    }


    return NULL;
}

int hash_table_del (hash_table_t *tbl, const char *key)
{
    hash_t h = hash(key);
    hash_table_entry_t **oe = &(tbl->entries[h % tbl->size]),
                        *e = *oe,
                        *e_del;
    int rc = 0;


    /* does the list contain anything at all? */
    if (e)
    {
        /* is it first? */
        if (!strcmp(e->key, key))
        {
            e_del = e;
            *oe = e->next;
            free(e_del);

            rc = 1;
        }
        else
        {
            while (e->next)
            {
                if (!strcmp(e->next->key, key))
                {
                    e_del = e->next;
                    e->next = e->next->next;
                    free(e_del);

                    rc = 1;
                    break;
                }

                e = e->next;
            }
        }
    }


    if (rc)
    {
        --tbl->count;

        /* If the table's load average is below the configured minimum
         * threshold, half its size */
        if (hash_table_get_load_factor(tbl) < tbl->min_load)
        {
            trce("hash table load factor too low, halving size to %u\n", tbl->size / 2);
            hash_table_resize(tbl, tbl->size / 2);
        }
    }
    else
    {
        /* FIXME: while debugging direntry_cache issues, it's useful to check
         * we're not trying to remove something that doesn't exist */
        assert(0);
    }


    return rc;
}

unsigned hash_table_get_size (hash_table_t *tbl)
{
    return tbl->size;
}

unsigned hash_table_get_count (hash_table_t *tbl)
{
    return tbl->count;
}

double hash_table_get_load_factor (hash_table_t *tbl)
{
    return (double)tbl->count / (double)tbl->size;
}

/* Resize entries table to new_size. Add all entries from, and free, the old
 * table. */
static void hash_table_resize (hash_table_t *tbl, unsigned new_size)
{
    hash_table_entry_t **entries = (hash_table_entry_t **)calloc(sizeof(hash_table_entry_t *) * new_size, 1),
                       *e, *e_prev;
    unsigned count = 0, i;


    if (!entries) return;


    for (i = 0; i < tbl->size; ++i)
    {
        e_prev = NULL;
        for (e = tbl->entries[i]; e; e = e->next)
        {
            add_to_entries(
                entries,
                new_size,
                e->key,
                e->data
            );
            free(e_prev);
            e_prev = e;
            ++count;
        }
        free(e_prev);
    }

    assert(count == tbl->count);
    free(tbl->entries);
    tbl->size    = new_size;
    tbl->entries = entries;
}


/* hash table iterator - public functions */

hash_table_iterator_t *hash_table_iterator_new (hash_table_t *tbl)
{
    hash_table_iterator_t *iter =
        (hash_table_iterator_t *)calloc(sizeof(hash_table_iterator_t), 1);


    iter->tbl = tbl;
    hash_table_iterator_find_first(iter);


    return iter;
}

void hash_table_iterator_delete (hash_table_iterator_t *iter)
{
    free(iter);
}

const char *hash_table_iterator_current_key (hash_table_iterator_t *iter)
{
    return (iter->end) ? NULL : iter->entry->key;
}

void *hash_table_iterator_current_data (hash_table_iterator_t *iter)
{
    return (iter->end) ? NULL : iter->entry->data;
}

int hash_table_iterator_next (hash_table_iterator_t *iter)
{
    hash_table_t *tbl = iter->tbl;
    unsigned s = iter->slot + 1;
    hash_table_entry_t *e = iter->entry;


    assert(e);

    if (e->next)
    {
        iter->entry = e->next;
        return 1;
    }

    while (s < tbl->size && !(e = tbl->entries[s]))
    {
        s++;
    }

    if (s < tbl->size)
    {
        iter->slot = s;
        iter->entry = e;
        return 1;
    }
    else
    {
        iter->end = 1;
        return 0;
    }
}

int hash_table_iterator_at_end (hash_table_iterator_t *iter)
{
    return iter->end;
}

/* hash table iterator - helpers */

static void hash_table_iterator_find_first (hash_table_iterator_t *iter)
{
    hash_table_t *tbl = iter->tbl;
    unsigned s = 0;
    hash_table_entry_t *e;


    while (!(e = tbl->entries[s++]) && s < tbl->size);

    if (s < tbl->size)
    {
        iter->slot = s - 1;
        iter->entry = e;
    }
    else
    {
        iter->end = 1;
    }


    return;
}


/* hash functions */

static hash_t hash_djb2 (const char *str)
{
    hash_t hash = 5381;

    while (*str)
    {
        hash = ((hash << 5) + hash) + *str; /* hash * 33 + c */
        str++;
    }

    return hash;
}

/* hash in use */
static hash_t (*hash) (const char *str) = &hash_djb2;


#if DEBUG
/* hash table - debugging */
static void hash_table_dump_histogram (hash_table_t *tbl)
{
    unsigned i;
    hash_table_entry_t *e;


    printf("hash table %p. size: %u, entries: %u, load factor: %f\n",
       (void *)tbl,
       hash_table_get_size(tbl),
       hash_table_get_count(tbl),
       (double)hash_table_get_load_factor(tbl)
    );

    for (i = 0; i < tbl->size; ++i)
    {
        printf("[%03u] ", i);

        e = tbl->entries[i];
        while (e)
        {
            printf("*");

            e = e->next;
        }

        printf("\n");
    }
}

static void hash_table_dump (hash_table_t *tbl)
{
    unsigned i;
    hash_table_entry_t *e;


    printf("hash table %p. size: %u, entries: %u, load factor: %f\n",
       (void *)tbl,
       hash_table_get_size(tbl),
       hash_table_get_count(tbl),
       (double)hash_table_get_load_factor(tbl)
    );

    for (i = 0; i < tbl->size; ++i)
    {
        printf("[%03u]\n", i);

        for (e = tbl->entries[i]; e; e = e->next)
        {
            printf("  %s\n", e->key);
        }
    }
}

static void hash_table_dump_row (hash_table_entry_t *e)
{
    for ( ; e; e = e->next)
    {
        printf("[entry %p] key: %s; data: %p\n",
            (void *)e,
            e->key,
            e->data
        );
    }
}

void hash_table_dump_dot (hash_table_t *tbl)
{
    unsigned i;
    hash_table_entry_t *e;


    for (i = 0; i < tbl->size; ++i)
    {
        if (tbl->entries[i]) printf("\"%03u\" [color=red]\n", i);
        for (e = tbl->entries[i]; e; e = e->next)
        {
            printf("\"%03u\" -> \"%s\" [color=red]\n", i, e->key);
        }
    }
}
#endif
