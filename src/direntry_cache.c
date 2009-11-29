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
#include <errno.h>
#include <string.h>
#include <parser.h>

#include "common.h"
#include "hash.h"
#include "locks.h"
#include "config.h"
#include "alarms.h"
#include "trace.h"
#include "fetcher.h"
#include "direntry.h"
#include "direntry_internal.h"
#include "direntry_cache.h"


TRACE_DEFINE(direntry_cache)


static hash_table_t *direntry_cache = NULL;
static rw_lock_t *direntry_cache_lock = NULL;


static void direntry_cache_del_tree (CALLER_DECL direntry_t *de);
static void direntry_cache_del_descendants (CALLER_DECL direntry_t *de);
static int direntry_cache_is_stale (direntry_t *de);
static int direntry_cache_is_expired (direntry_t *de);


int direntry_cache_init (void)
{
    direntry_t *de_root;


    direntry_cache_trace("direntry_cache_init()\n");

    direntry_cache_lock = rw_lock_new();

    direntry_cache = hash_table_new(
        16,
        config_tuning_direntry_cache_max_load,
        config_tuning_direntry_cache_min_load
    );

    /* The cache has a permanent root node, made here, as it can't be fetched */
    de_root = direntry_new_root(CALLER_INFO_ONLY);
    direntry_cache_add(CALLER_INFO de_root);
    direntry_delete(CALLER_INFO de_root);


    return 0;
}

void direntry_cache_finalise (void)
{
    direntry_t *de_root;


    direntry_cache_trace("direntry_cache_finalise()\n");

    assert(direntry_cache_get(CALLER_INFO "/", &de_root) == direntry_cache_status_HIT);
    direntry_delete(CALLER_INFO de_root);
    direntry_cache_del_tree(CALLER_INFO de_root);

    hash_table_delete(direntry_cache);
    direntry_cache = NULL;

    rw_lock_delete(direntry_cache_lock);
}

int direntry_cache_add (CALLER_DECL direntry_t *de)
{
#if DEBUG
    direntry_t *de2;
#endif


    direntry_cache_trace("[direntry %p] cache add (" CALLER_FORMAT ")\n",
                         de, CALLER_PASS_ONLY);
    direntry_cache_trace_indent();

#if DEBUG
    rw_lock_rlock(direntry_cache_lock);
    de2 = hash_table_find(direntry_cache, direntry_get_path(de));
    rw_lock_runlock(direntry_cache_lock);
    assert(!de2); /* Shouldn't be duplicates */
#endif


    direntry_post(CALLER_INFO de); /* inc reference count */

    de->cache_last_valid = time(NULL);

    rw_lock_wlock(direntry_cache_lock);
    hash_table_add(direntry_cache, direntry_get_path(de), (void *)de);
    rw_lock_wunlock(direntry_cache_lock);

    direntry_cache_trace_dedent();


    return 0;
}

void direntry_cache_add_list (direntry_t *dirents, const char *parent)
{
    direntry_t *de, *de_parent;


    assert(direntry_cache_get(CALLER_INFO parent, &de_parent) == direntry_cache_status_HIT);

    for (de = dirents; de; de = direntry_get_next_sibling(de))
    {
        de->parent = de_parent;
        direntry_cache_add(CALLER_INFO de);
    }

    direntry_set_looked_for_children(de_parent, 1);
    de_parent->children = dirents;
}

void direntry_cache_add_children (
    direntry_t *parent,
    direntry_t *new_children
)
{
    direntry_t *old_children = parent->children, *new_child, *old_child;
    int found;


    rw_lock_wlock(direntry_cache_lock);

    parent->children = new_children;

    /* Transfer ownership of all child subtrees from old list to new list */
    for (new_child = new_children; new_child; new_child = new_child->next)
    {
        for (old_child = old_children; old_child; old_child = old_child->next)
        {
            if (!strcmp(direntry_get_path(new_child), direntry_get_path(old_child)))
            {
                new_child->children = old_child->children;
            }
        }
    }

    /* Free the old list and any remaining subtrees that weren't carried over */
    for (old_child = old_children; old_child; old_child = old_child->next)
    {
        found = 0;
        for (new_child = new_children; new_child; new_child = new_child->next)
        {
            if (!strcmp(direntry_get_path(new_child), direntry_get_path(old_child)))
            {
                found = 1;
                break;
            }
        }

        /* Any children of old_child have not been usurped by something in the
         * new list - discard them */
        if (!found)
        {
            direntry_cache_del_descendants(CALLER_INFO old_child);
        }

        direntry_cache_del(CALLER_INFO old_child);
    }

    /* Finally, claim cache ownership of the new list (but not until the old
     * ones have been free()d, so that the cache doesn't contain duplicates */
    for (new_child = new_children; new_child; new_child = new_child->next)
    {
        new_child->parent = parent;
        direntry_cache_add(CALLER_INFO new_child);
    }


    rw_lock_wunlock(direntry_cache_lock);
}

/* Get the de for path out of the cache, else return NULL */
direntry_cache_status_t direntry_cache_get (CALLER_DECL const char * const path, direntry_t **de_out)
{
    direntry_t *de = NULL, *de_parent;
    direntry_cache_status_t stat = direntry_cache_status_UNKNOWN;
    const char *parent = fsfuse_dirname(path);


    rw_lock_rlock(direntry_cache_lock);
    de = (direntry_t *)hash_table_find(direntry_cache, path);
    rw_lock_runlock(direntry_cache_lock);

    direntry_cache_trace("direntry_cache_get(%s) (" CALLER_FORMAT "): %s\n",
            path, CALLER_PASS (de ? "hit" : "miss"));
    direntry_cache_trace_indent();

    if (de)
    {
        /* check for expiry */
        if (direntry_cache_is_expired(de))
        {
            direntry_cache_trace("cache entry found to be stale\n");

            direntry_cache_notify_stale(de);
            de = NULL;
        }

        if (de)
        {
            stat = direntry_cache_status_HIT;
            direntry_post(CALLER_PASS de); /* inc reference count */
        }
    }
    else
    {
        rw_lock_rlock(direntry_cache_lock);
        de_parent = (direntry_t *)hash_table_find(direntry_cache, parent);
        rw_lock_runlock(direntry_cache_lock);

        if (de_parent && direntry_get_looked_for_children(de_parent))
        {
            stat = direntry_cache_status_NOENT;
        }
    }

    free((char *)parent);
    direntry_cache_trace_dedent();


    if (de_out) *de_out = de;
    return stat;
}

int direntry_cache_del (CALLER_DECL direntry_t *de)
{
    int rc;


    direntry_cache_trace("[direntry %p] cache delete (" CALLER_FORMAT ")\n",
                         de, CALLER_PASS_ONLY);

    rw_lock_wlock(direntry_cache_lock);
    rc = hash_table_del(direntry_cache, direntry_get_path(de));
    rw_lock_wunlock(direntry_cache_lock);

    assert(direntry_cache_get(CALLER_INFO direntry_get_path(de), NULL) != direntry_cache_status_HIT);

    direntry_delete(CALLER_PASS de);


    return rc;
}

void direntry_cache_notify_still_valid (direntry_t *de)
{
    de->cache_last_valid = time(NULL);
}

void direntry_cache_notify_stale (direntry_t *de)
{
    direntry_t *parent;


    direntry_cache_trace("direntry_cache_notify_stale(%s)\n", direntry_get_path(de));
    direntry_cache_trace_indent();

    if (direntry_is_root(de))
    {
        parent = de;
    }
    else
    {
        parent = direntry_get_parent(de);
    }

    while (!direntry_is_root(parent) &&
           direntry_cache_is_stale(parent)
          )
    {
        parent = direntry_get_parent(parent);
    }

    direntry_cache_trace("decided everything up to %s was stale\n",
            direntry_get_path(parent));


    /* parent is fine... */
    direntry_cache_notify_still_valid(parent);

    /* ...but one of its children is stale. Assume all children are
     * stale and remove them. */
    direntry_cache_del_descendants(CALLER_INFO parent);
    parent->children = NULL;


    direntry_cache_trace_dedent();

}

/* Delete the tree of direntries anchored at this node. */
/* WARNING: internal function - this does not clean up any parent / sibling
 * lists, or any other metadata, that the root node is a member of */
static void direntry_cache_del_tree (CALLER_DECL direntry_t *de)
{
    direntry_t *child = direntry_get_first_child(de), *next_child;


    direntry_cache_trace("direntry_cache_del_tree(%s)\n", direntry_get_path(de));

    while (child)
    {
        next_child = direntry_get_next_sibling(child);
        direntry_cache_del_tree(CALLER_PASS child);
        child = next_child;
    }

    /* actually remove this de from the cache */
    direntry_cache_del(CALLER_PASS de);
}

/* Recursively delete all of this node's children, but *not* the node itself */
static void direntry_cache_del_descendants (CALLER_DECL direntry_t *de)
{
    direntry_t *child = direntry_get_first_child(de), *next_child;


    while (child)
    {
        next_child = direntry_get_next_sibling(child);
        direntry_cache_del_tree(CALLER_PASS child);
        child = next_child;
    }
}

/* Is this direntry "stale"? That is, do we no longer trust it to (probably)
 * reflect accurately on the state of things. Reasons we may no longer trust a
 * direntry include:
 * - Upon querying the server, that path no longer exists.
 * - The de describes a directory, but is so old that it's reasonable to assume
 *   that children may be been added or removed
 */
static int direntry_cache_is_stale (direntry_t *de)
{
    int rc = 0;


    /* Has it expired? */
    if (direntry_cache_is_expired(de))
    {
        rc = 1;
    }
    else
    {
        /* Does it no longer exist? */
        rc = parser_fetch_listing(direntry_get_path(de),
                                  NULL
                                 );

        if (rc) rc = 1;
    }

    direntry_cache_trace("direntry_cache_is_stale(%s): %s\n", direntry_get_path(de), (rc) ? "yes" : "no");


    return rc;
}

static int direntry_cache_is_expired (direntry_t *de)
{
    if (direntry_is_root(de)) return 0; /* "/" never goes stale */

    return de->cache_last_valid < time(NULL) - config_timeout_cache;
}

#if DEBUG
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
            (direntry_get_type(de) == direntry_type_DIRECTORY && !direntry_get_looked_for_children(de)) ? "green" : "blue"
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

/* Render path's cache status */
char *direntry_cache_status (const char * const path)
{
    direntry_t *de = NULL;
    char *ret = (char *)malloc(1024);


    ret[0] = '\0';

    rw_lock_rlock(direntry_cache_lock);
    de = (direntry_t *)hash_table_find(direntry_cache, path);
    rw_lock_runlock(direntry_cache_lock);

    if (de)
    {
        strcat(ret, " CACHED");

        /* check for expiry */
        if (direntry_cache_is_expired(de))
        {
            strcat(ret, " STALE");
        }
    }


    return ret;
}
#endif
