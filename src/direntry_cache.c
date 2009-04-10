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


void direntry_cache_init (void)
{
    direntry_trace("direntry_cache_init()\n");

    rw_lock_init(&direntry_cache_lock);

    direntry_cache = hash_table_new(DIRENTRY_CACHE_SIZE);
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
    direntry_t *de = NULL;


    rw_lock_rlock(&direntry_cache_lock);
    de = (direntry_t *)hash_table_find(direntry_cache, path);
    rw_lock_runlock(&direntry_cache_lock);

    /* You may think that it's an obvious step here to see if we've got its
     * parent cached. If this is the case, and the parent's got a child list,
     * yet we failed to find the direntry in the cache, then it would be
     * tempting to assert that it doesn't exist. THIS IS NOT THE CACHE'S JOB.
     * Just like the cache does not store a "negative" entry when we look up a
     * path and don't find it, the cache cannot assert the non-existence of a
     * direntry. After all, it's just a cache, and in our case it can't know if
     * it's out of date. */

    direntry_trace("direntry_cache_get(path==%s): %s\n",
            path, (de ? "hit" : "miss"));

    if (de)
    {
        direntry_post(de); /* inc reference count */
    }


    return de;
}

int direntry_cache_del (direntry_t *de)
{
    int rc;


    direntry_trace("direntry_cache_del(path==%s)\n",
                   direntry_get_path(de));

    rw_lock_wlock(&direntry_cache_lock);
    rc = hash_table_del(direntry_cache, direntry_get_path(de));
    rw_lock_wunlock(&direntry_cache_lock);

    direntry_delete(de);


    return rc;
}

void direntry_cache_notify_stale (direntry_t *de)
{
    direntry_cache_del(de);
}
