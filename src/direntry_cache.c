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
#include <errno.h>

#include "common.h"
#include "hash.h"
#include "locks.h"
#include "config.h"
#include "alarms.h"
#include "trace.h"
#include "fetcher.h"
#include "direntry.h"
#include "direntry_cache.h"


TRACE_DEFINE(direntry_cache)

#define CACHE_EMPTY_TIMEOUT 1 * 60
#define DIRENTRY_CACHE_SIZE 16


static hash_table_t *direntry_cache = NULL;
static rw_lock_t direntry_cache_lock;


static int direntry_cache_is_stale (direntry_t *de);
static void direntry_cache_del_descendants (direntry_t *de);


void direntry_cache_init (void)
{
    direntry_cache_trace("direntry_cache_init()\n");

    rw_lock_init(&direntry_cache_lock);

    direntry_cache = hash_table_new(DIRENTRY_CACHE_SIZE);
}

void direntry_cache_finalise (void)
{
    direntry_cache_trace("direntry_cache_finalise()\n");

    hash_table_delete(direntry_cache);
    direntry_cache = NULL;

    rw_lock_destroy(&direntry_cache_lock);
}

int direntry_cache_add (direntry_t *de)
{
    direntry_t *parent;


    direntry_cache_trace("direntry_cache_add(de->base_name==\"%s\",->path==\"%s\")\n",
                   direntry_get_base_name(de),
                   direntry_get_path(de) );
    direntry_cache_trace_indent();


    rw_lock_rlock(&direntry_cache_lock);
    parent = hash_table_find(direntry_cache, direntry_get_path(de));
    rw_lock_runlock(&direntry_cache_lock);
    assert(!parent); /* Shouldn't be duplicates */


    if (!direntry_is_root(de))
    {
        /* parent *must* exist! */
        rw_lock_rlock(&direntry_cache_lock);
        parent = hash_table_find(direntry_cache, fsfuse_dirname(direntry_get_path(de)));
        rw_lock_runlock(&direntry_cache_lock);
        assert(parent);

        de->parent = parent;
        de->next = parent->children;
        parent->children = de;
    }


    direntry_post(de); /* inc reference count */

    de->cache_last_valid = time(NULL);

    rw_lock_wlock(&direntry_cache_lock);
    hash_table_add(direntry_cache, direntry_get_path(de), (void *)de);
    rw_lock_wunlock(&direntry_cache_lock);

    direntry_cache_trace_dedent();


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

    direntry_cache_trace("direntry_cache_get(path==%s): %s\n",
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


    direntry_cache_trace("direntry_cache_del(path==%s)\n",
                   direntry_get_path(de));

    rw_lock_wlock(&direntry_cache_lock);
    rc = hash_table_del(direntry_cache, direntry_get_path(de));
    rw_lock_wunlock(&direntry_cache_lock);

    direntry_delete(de);


    return rc;
}

void direntry_cache_notify_stale (direntry_t *de)
{
    direntry_t *parent = direntry_get_parent(de);


    direntry_cache_trace("direntry_notify_stale(%s)\n", direntry_get_path(de));
    direntry_cache_trace_indent();

    while (!direntry_is_root(de) &&
           direntry_cache_is_stale(parent)
          )
    {
        parent = direntry_get_parent(parent);
    }

    direntry_cache_trace("decided everything up to %s was stale\n",
            direntry_get_path(parent));


    /* purge from cache*/
    direntry_cache_del_descendants(parent);


    direntry_cache_trace_dedent();

}

/* Delete the tree of direntries anchored at this node. */
/* WARNING: internal function - this does not clean up any parent / sibling
 * lists, or any other metadata, that the root node is a member of */
static void direntry_cache_del_tree (direntry_t *de)
{
    direntry_t *child = direntry_get_first_child(de);


    while (child)
    {
        direntry_cache_del_tree(child);
        child = direntry_get_next_sibling(child);
    }

    /* actually remove this de from the cache */
    direntry_cache_del(de);
}

/* Recursively delete all of this node's children, but *not* the node itself */
static void direntry_cache_del_descendants (direntry_t *de)
{
    direntry_t *child = direntry_get_first_child(de);


    while (child)
    {
        direntry_cache_del_tree(child);
        child = direntry_get_next_sibling(child);
    }

    /* This may look like a horrible hack - nulling this stuff out on the
     * assumption that deleting the children will cause them to be free()d. It's
     * actually not that: it doesn't matter if the children aren't free()d, the
     * point here is that this tree structure is only maintained over direntries
     * in the cache, which these are no longer. Re-adding them (in the correct
     * order) would cause this info to be build again. */
    de->children = NULL;
    de->looked_for_children = 0;
}

/* Is this direntry "stale". That is, do we no longer trust it to (probably)
 * reflect accurately on the state of things. Reasons we may no longer trust a
 * direntry include:
 * - Upon querying the server, that path no longer exists.
 * - The de describes a directory, but is so old that it's reasonable to assume
 *   that children may be been added or removed
 */
static int direntry_cache_is_stale (direntry_t *de)
{
    int rc = 0;


    direntry_cache_trace("direntry_is_stale(%s)\n", direntry_get_path(de));
    direntry_cache_trace_indent();

    rc = fetcher_fetch(direntry_get_path(de),
                       fetcher_url_type_t_BROWSE,
                       NULL,
                       NULL,
                       NULL
                      );

    if (rc)
    {
        assert(rc == -ENOENT);

        rc = 1;
    }

    direntry_cache_trace("%s\n", (rc) ? "yes" : "no");
    direntry_cache_trace_dedent();


    return rc;
}


/* debug functions */
static void direntry_cache_dump_tree (direntry_t *de)
{
    static unsigned indent = 0;
    unsigned i;
    direntry_t *child;


    for (i = 0; i < indent; ++i) printf("  ");
    printf("%s\n", direntry_get_path(de));

    ++indent;
    for (child = direntry_get_first_child(de); child; child = direntry_get_next_sibling(de))
    {
        direntry_cache_dump_tree(child);
    }
}

static void direntry_cache_dump_tree_dot_do (direntry_t *parent, direntry_t *de)
{
    direntry_t *child;


    printf("\"%s\" [color=%s]\n",
            direntry_get_path(de),
            (direntry_get_type(de) == direntry_type_DIRECTORY && !direntry_got_children(de)) ? "green" : "blue"
          );
    if (parent)
    {
        printf("\"%s\" -> \"%s\" [color=blue]\n", direntry_get_path(parent), direntry_get_path(de));
    }

    for (child = direntry_get_first_child(de); child; child = direntry_get_next_sibling(child))
    {
        direntry_cache_dump_tree_dot_do(de, child);
    }
}

static void direntry_cache_dump_tree_dot (direntry_t *de)
{
    direntry_cache_dump_tree_dot_do(NULL, de);
}

/* special, amalgamated view of both tree structure and hash table structure */
static void direntry_cache_dump_dot (void)
{
    direntry_t *de_root = hash_table_find(direntry_cache, "/");


    hash_table_dump_dot(direntry_cache);

    direntry_cache_dump_tree_dot(de_root);
}
