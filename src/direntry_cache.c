/*
 * Caching mechanism for direntrys and direntry trees.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include "uthash.h"

#include "common.h"
#include "locks.h"
#include "config.h"
#include "alarms.h"
#include "trace.h"
#include "direntry.h"
#include "direntry_cache.h"


#define CACHE_EMPTY_TIMEOUT 1 * 60


static direntry_t *direntry_cache = NULL;
static rw_lock_t direntry_cache_lock;


static void cache_empty_cb (void *ctxt);
static void sigusr1_handler (int signum);


void direntry_cache_init (void)
{
    direntry_trace("direntry_cache_init()\n");

    rw_lock_init(&direntry_cache_lock);

    signal(SIGUSR1, &sigusr1_handler);

    alarm_schedule(CACHE_EMPTY_TIMEOUT, 0, &cache_empty_cb, NULL);
}

void direntry_cache_finalise (void)
{
    direntry_trace("direntry_cache_finalise()\n");

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
    HASH_ADD_KEYPTR(hh,
                    direntry_cache,
                    direntry_get_path(de),
                    strlen(direntry_get_path(de)),
                    de); /* FIXME: segfault seem here */
    rw_lock_wunlock(&direntry_cache_lock);

    direntry_trace_dedent();


    return 0;
}

/* Get the de for path out of the cache, else return NULL */
direntry_t *direntry_cache_get (const char * const path)
{
    direntry_t *de = NULL, *parent_de = NULL;


    rw_lock_rlock(&direntry_cache_lock);
    HASH_FIND(hh, direntry_cache, path, (signed)strlen(path), de); /* FIXME segfault seen here */
    rw_lock_runlock(&direntry_cache_lock);

    if (!de)
    {
        /* do we have its parent cached? */
        char *parent = fsfuse_dirname(path);

        rw_lock_rlock(&direntry_cache_lock);
        HASH_FIND(hh, direntry_cache, parent, (signed)strlen(parent), parent_de);
        rw_lock_runlock(&direntry_cache_lock);

        free(parent);

        if (parent_de && !parent_de->looked_for_children)
        {
            populate_directory(parent_de);

            rw_lock_rlock(&direntry_cache_lock);
            HASH_FIND(hh, direntry_cache, path, (signed)strlen(path), de);
            rw_lock_runlock(&direntry_cache_lock);
        }
    }

#if 0
    /* still valid? */
    if (de && de->cache_last_valid + config_get(config_key_CACHE_ITEM_TIMEOUT).int_val < time(NULL))
    {
        direntry_trace("cache entry for %s expired; deleting\n", direntry_get_base_name(de));
        HASH_DELETE(hh, direntry_cache, de);
        direntry_delete(de);
        de = NULL;
    }
#endif

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
    direntry_t *de;


    rw_lock_wlock(&direntry_cache_lock);

    assert(signum == SIGUSR1);
    direntry_trace("sigusr1_handler()\n");
    direntry_trace_indent();
    direntry_trace("removing all %u items from the direntry cache\n",
                   HASH_COUNT(direntry_cache));

    while (direntry_cache)
    {
        de = direntry_cache;
        HASH_DEL(direntry_cache, de); /* FIXME: segfault seen here */
        direntry_delete(de);
    }

    /* horrible special case - freeing everything means we have to unset the
     * children got flag on "/" */
    de_root->looked_for_children = 0;
    de_root->children = NULL;

    assert(HASH_COUNT(direntry_cache) == 0);

    rw_lock_wunlock(&direntry_cache_lock);

    direntry_trace_dedent();
}

#if 0
            /* Not in the cache. Recurse to find its parent. We don't check for a
             * stopping condition because "/" is always in the cache.
             * It could be that its parent doesn't exist, in which case we bail out */
            rc = direntry_get(path_parent(path), de);
            if (!rc)
            {
                /* If the parent doesn't have any children, go and get them */
                if (!(*de)->children)
                {
                    /* Parent has no children => hasn't been populated. Populate */
                    direntry_populate_directory(*de);
                }
                /* Look again in the cache, now all of path's parent's children are
                 * there. If it's still not there, it doesn't exist */
                *de = direntry_cache_get(path);
                if (!*de) rc = -ENOENT;
            }
        }
#endif
