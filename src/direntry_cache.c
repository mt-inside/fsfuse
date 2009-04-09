/*
 * Caching mechanism for direntrys and direntry trees.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

/* FIXME: this needs a total re-write. E.g. the locking's all wrong - a single
 * search involving multiple finds of different paths should be atomic, yes? */

#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include "common.h"
#include "hash.h"
#include "locks.h"
#include "config.h"
#include "alarms.h"
#include "trace.h"
#include "direntry.h"
#include "direntry_cache.h"


#define CACHE_EMPTY_TIMEOUT 1 * 60
#define DIRENTRY_CACHE_SIZE 256


static hash_table_t *direntry_cache = NULL;
static rw_lock_t direntry_cache_lock;


static void cache_empty_cb (void *ctxt);
static void sigusr1_handler (int signum);


void direntry_cache_init (void)
{
    direntry_trace("direntry_cache_init()\n");

    rw_lock_init(&direntry_cache_lock);

    direntry_cache = hash_table_new(DIRENTRY_CACHE_SIZE);

    signal(SIGUSR1, &sigusr1_handler);

    alarm_schedule(CACHE_EMPTY_TIMEOUT, 0, &cache_empty_cb, NULL);
}

void direntry_cache_finalise (void)
{
    direntry_trace("direntry_cache_finalise()\n");

    hash_table_delete(direntry_cache);
    direntry_cache = NULL;

    rw_lock_destroy(&direntry_cache_lock);
}

int direntry_cache_add (direntry_t *de)
{
    direntry_trace("direntry_cache_add(de->base_name==\"%s\",->path==\"%s\")\n",
                   direntry_get_base_name(de),
                   direntry_get_path(de) );
    direntry_trace_indent();

    direntry_post(de); /* inc reference count */

    de->cache_last_valid = time(NULL);

    rw_lock_wlock(&direntry_cache_lock);
    hash_table_add(direntry_cache, direntry_get_path(de), (void *)de);
    rw_lock_wunlock(&direntry_cache_lock);

    direntry_trace_dedent();


    return 0;
}

/* Get the de for path out of the cache, else return NULL */
direntry_t *direntry_cache_get (const char * const path)
{
    direntry_t *de = NULL, *parent_de = NULL;


    rw_lock_rlock(&direntry_cache_lock);
    de = (direntry_t *)hash_table_find(direntry_cache, path);
    rw_lock_runlock(&direntry_cache_lock);

    if (!de)
    {
        /* do we have its parent cached? */
        char *parent = fsfuse_dirname(path);

        rw_lock_rlock(&direntry_cache_lock);
        de = (direntry_t *)hash_table_find(direntry_cache, path);
        rw_lock_runlock(&direntry_cache_lock);

        free(parent);

        if (parent_de && !parent_de->looked_for_children)
        {
            populate_directory(parent_de);

            rw_lock_rlock(&direntry_cache_lock);
            de = (direntry_t *)hash_table_find(direntry_cache, path);
            rw_lock_runlock(&direntry_cache_lock);
        }
    }

    direntry_trace("direntry_cache_get(path==%s): %s\n",
            path, (de ? "hit" : "miss"));

    if (de)
    {
        direntry_post(de); /* ref_count++ */
    }


    return de;
}

static void cache_empty_cb (void *ctxt)
{
    NOT_USED(ctxt);


    sigusr1_handler(SIGUSR1);

    alarm_schedule(CACHE_EMPTY_TIMEOUT, 0, &cache_empty_cb, NULL);
}

static void sigusr1_handler (int signum)
{
    hash_table_iterator_t *iter;
    const char *key = NULL;


    rw_lock_wlock(&direntry_cache_lock);

    assert(signum == SIGUSR1);
    direntry_trace("sigusr1_handler()\n");
    direntry_trace_indent();
    direntry_trace("removing all %u items from the direntry cache\n",
                   hash_table_get_count(direntry_cache));


    iter = hash_table_iterator_new(direntry_cache);

    /* hash table iterator is NOT delete-safe, hence this complicated loop
     * structure */
    while (!hash_table_iterator_at_end(iter))
    {
        if (key)
        {
            hash_table_del(direntry_cache, key);
        }
        key = hash_table_iterator_current_key(iter);

        hash_table_iterator_next(iter);
    }
    if (key)
    {
        hash_table_del(direntry_cache, key);
    }

    hash_table_iterator_delete(iter);


    /* horrible special case - freeing everything means we have to unset the
     * children got flag on "/" */
    de_root->looked_for_children = 0;
    de_root->children = NULL;

    assert(!hash_table_get_count(direntry_cache));

    rw_lock_wunlock(&direntry_cache_lock);

    direntry_trace_dedent();
}
