#include "common.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>

#include "hash.h"


typedef uint32_t hash_t;


static hash_t (*hash) (const char *str);

static void hash_table_iterator_find_first (hash_table_iterator_t *iter);


/* hash table - public functions */

hash_table_t *hash_table_new (unsigned size)
{
    hash_table_t *tbl = (hash_table_t *)malloc(sizeof(hash_table_t));


    tbl->size = size;
    tbl->entries = (hash_table_entry_t **)calloc(sizeof(hash_table_entry_t *) * tbl->size, 1);


    return tbl;
}

void hash_table_delete (hash_table_t *tbl)
{
    assert(!tbl->count);
    free(tbl->entries);
    free(tbl);
}

unsigned hash_table_get_count (hash_table_t *tbl)
{
    return tbl->count;
}

void hash_table_add (hash_table_t *tbl, const char *key, void *data)
{
    hash_t h = hash(key);
    hash_table_entry_t  *e  = (hash_table_entry_t *)malloc(sizeof(hash_table_entry_t)),
                       **oe = &(tbl->entries[h % tbl->size]);


    e->key  = key;
    e->data = data;
    e->next = *oe;

    *oe = e;

    ++tbl->count;
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
    hash_table_entry_t **e = &(tbl->entries[h % tbl->size]),
                        *e_del;
    int rc = 0;


    /* does the list contain anything at all? */
    if (*e)
    {
        /* is it first? */
        if (!strcmp((*e)->key, key))
        {
            e_del = *e;
            *e = (*e)->next;
            free(e_del);

            rc = 1;
        }
        else
        {
            while ((*e)->next)
            {
                if (!strcmp((*e)->next->key, key))
                {
                    e_del = (*e)->next;
                    (*e)->next = (*e)->next->next;
                    free(e_del);

                    rc = 1;
                }

                *e = (*e)->next;
            }
        }
    }


    if (rc)
    {
        --tbl->count;
    }


    return rc;
}

/* hash table - helpers */

static void hash_table_dump_histogram (hash_table_t *tbl)
{
    unsigned i;
    hash_table_entry_t *e;


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

    while (!(e = tbl->entries[s++]) && s < tbl->size);

    if (s < tbl->size || e)
    {
        iter->slot = s - 1;
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

static hash_t hash_sha (const char *str)
{
    unsigned char digest[SHA_DIGEST_LENGTH];


    SHA1((unsigned char *)str, strlen(str), &digest);


    return *((hash_t *)&digest);
}
static hash_t hash_djb2 (const char *str)
{
    hash_t hash = 5381;
    char c;

    while ((c = *str++))
    {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}

/* hash in use */
static hash_t (*hash) (const char *str) = &hash_djb2;

