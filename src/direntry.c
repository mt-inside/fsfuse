/*
 * This file deals with "direntries" - "entries in a directory". This is
 * basically our internal file system tree implementation.
 *
 * Copyright (C) Matthew Turner 2008-2010. All rights reserved.
 *
 * $Id$
 */

#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "config.h"
#include "direntry.h"
#include "fetcher.h"
#include "inode_map.h"
#include "listing.h"
#include "parser.h"
#include "string_buffer.h"


TRACE_DEFINE(direntry)


struct _direntry_t
{
    listing_t          *li;

    unsigned            ref_count;
    pthread_mutex_t    *lock;

    ino_t               inode;
    struct _direntry_t *next;
    struct _direntry_t *parent;
    struct _direntry_t *children;
    int                 looked_for_children;
};


static direntry_t *direntry_new_root (CALLER_DECL_ONLY);
static direntry_t *direntry_from_listing (CALLER_DECL listing_t *li);
static direntry_t *direntries_from_listing_list (listing_list_t *lis, direntry_t *parent);


/* direntry module external interface ======================================= */

int direntry_init (void)
{
    direntry_trace("direntry_init()\n");


    /* TODO: horrid way to get this in the inode map (which takes a copy) */
    direntry_delete(CALLER_INFO direntry_new_root(CALLER_INFO_ONLY));


    return 0;
}

void direntry_finalise (void)
{
    inode_map_clear();
}


/* Innards ================================================================== */

int direntry_ensure_children (
    direntry_t *de
)
{
    int rc = EIO;
    char *path, *url;
    listing_list_t *lis = NULL;
    direntry_t *dirents = NULL;


    if (!de->looked_for_children)
    {
        path = direntry_get_path(de);
        direntry_trace("direntry_get_children(%s)\n", path);


        /* fetch the directory listing from the indexnode */
        url = make_url("browse", path + 1);
        rc = parser_fetch_listing(url, &lis);
        free(path);
        free(url);

        if (!rc && lis)
        {
            dirents = direntries_from_listing_list (lis, de);

            listing_list_delete(CALLER_INFO lis);
        }

        de->children = dirents;
        de->looked_for_children = 1;
    }


    return rc;
}

static direntry_t *direntries_from_listing_list (listing_list_t *lis, direntry_t *parent)
{
    direntry_t *de = NULL, *prev = NULL;
    listing_t *li;
    unsigned i;


    assert(lis);

    /* turn array of li's into linked list of de's */
    for (i = 0; i < listing_list_get_count(lis); ++i)
    {
        li = listing_list_get_item(lis, i);
        de = direntry_from_listing(CALLER_INFO li);
        listing_delete(CALLER_INFO li);


        de->parent = parent;
        de->next = prev;
        prev = de;
    }


    return de;
}


/* direntry lifecycle ======================================================= */

static direntry_t *direntry_new (CALLER_DECL ino_t inode)
{
    direntry_t *de = (direntry_t *)calloc(1, sizeof(direntry_t));


    de->lock = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(de->lock, NULL);

    de->ref_count = 1;

    de->inode = inode;
    inode_map_add(de);

    direntry_trace(
        "[direntry %p] new (" CALLER_FORMAT ") ref %u inode %lu\n",
        de, CALLER_PASS de->ref_count, de->inode
    );


    return de;
}

static direntry_t *direntry_from_listing (CALLER_DECL listing_t *li)
{
    direntry_t *de = direntry_new(CALLER_PASS inode_next());


    de->li = listing_post(CALLER_PASS li);


    return de;
}

direntry_t *direntry_new_root (CALLER_DECL_ONLY)
{
    direntry_t *de = direntry_new(CALLER_PASS FSFUSE_ROOT_INODE);
    listing_t *li = listing_new(CALLER_PASS_ONLY);


    listing_attribute_add(li, "fs2-name", "");
    listing_attribute_add(li, "fs2-type", "directory");
    /* Set the link count to something non-0. We can't query this from the
     * indexnode. We could work it out every time, but that would be tedious.
     * It turns out to be vitally important that this is non-0 if samba is going
     * to share the filesystem */
    //TODO: calculate me in the future
    listing_attribute_add(li, "fs2-linkcount", "1");

    de->li = li;


    return de;
}

direntry_t *direntry_post (CALLER_DECL direntry_t *de)
{
#if DEBUG
    string_buffer_t *trace_str = string_buffer_new();
#endif /* DEBUG */


    assert(de->ref_count);

    pthread_mutex_lock(de->lock);
    ++de->ref_count;
    pthread_mutex_unlock(de->lock);

#if DEBUG
    string_buffer_printf(trace_str,
            "[direntry %p] post (" CALLER_FORMAT ") ref %u\n",
            (void *)de, CALLER_PASS de->ref_count);
    direntry_trace(string_buffer_peek(trace_str));
    string_buffer_delete(trace_str);
#endif /* DEBUG */


    return de;
}

void direntry_delete (CALLER_DECL direntry_t *de)
{
    unsigned refc;
#if DEBUG
    string_buffer_t *trace_str = string_buffer_new();


    string_buffer_printf(trace_str,
            "[direntry %p] delete (" CALLER_FORMAT ") ref %u\n",
            (void *)de, CALLER_PASS de->ref_count - 1);
    direntry_trace(string_buffer_peek(trace_str));
    string_buffer_delete(trace_str);
#endif /* DEBUG */

    assert(de->ref_count);

    pthread_mutex_lock(de->lock);
    refc = --de->ref_count;
    pthread_mutex_unlock(de->lock);

    direntry_trace_indent();

    if (!refc)
    {
        direntry_trace("refcount == 0 => free()ing\n");

        pthread_mutex_destroy(de->lock);
        free(de->lock);

        listing_delete(CALLER_INFO de->li);

        free(de);
    }

    direntry_trace_dedent();
}

void direntry_delete_list (CALLER_DECL direntry_t *de)
{
    direntry_t *next;

    while (de)
    {
        next = direntry_get_next_sibling(de);
        direntry_delete(CALLER_PASS de);
        de = next;
    }
}


/* direntry equality ======================================================== */

int direntry_equal (direntry_t *de, direntry_t *other)
{
    return listing_equal(de->li, other->li);
}


/* direntry tree traversal functions ======================================== */
/* for now, we only provide these simple wrappers around the underlying
 * implementation details. Iterator functions could be built from these */

direntry_t *direntry_get_parent       (direntry_t *de)
{
    return de->parent ? direntry_post(CALLER_INFO de->parent) : NULL;
}

direntry_t *direntry_get_first_child  (direntry_t *de)
{
    return de->children ? direntry_post(CALLER_INFO de->children) : NULL;
}

direntry_t *direntry_get_next_sibling (direntry_t *de)
{
    return de->next ? direntry_post(CALLER_INFO de->next) : NULL;
}


/* direntry attribute getters =============================================== */

ino_t direntry_get_inode (direntry_t *de)
{
    return de->inode;
}

static void direntry_get_path_inner (direntry_t *de, char *path)
{
    direntry_t *parent;


    if ((parent = direntry_get_parent(de)))
    {
        direntry_get_path_inner(parent, path);
        direntry_delete(CALLER_INFO parent);
    }

    strcat(path, direntry_get_name(de));
    strcat(path, "/");
}
char *direntry_get_path (direntry_t *de)
{
    /* FIXME */
    char *path = malloc(1024);


    path[0] = '\0';

    direntry_get_path_inner(de, path);


    return path;
}

char *direntry_get_name (direntry_t *de)
{
    return listing_get_name(de->li);
}

char *direntry_get_hash (direntry_t *de)
{
    return listing_get_hash(de->li);
}

direntry_type_t direntry_get_type (direntry_t *de)
{
    return listing_get_type(de->li);
}

off_t direntry_get_size (direntry_t *de)
{
    return listing_get_size(de->li);
}

unsigned long direntry_get_link_count (direntry_t *de)
{
    return listing_get_link_count(de->li);
}

char *direntry_get_href (direntry_t *de)
{
    return listing_get_href(de->li);
}

int direntry_get_looked_for_children (direntry_t *de)
{
    return de->looked_for_children;
}

void direntry_set_looked_for_children (direntry_t *de, int val)
{
    de->looked_for_children = val;
}

int direntry_is_root (direntry_t *de)
{
    return de->inode == FSFUSE_ROOT_INODE;
}

void direntry_de2stat (direntry_t *de, struct stat *st)
{
    listing_li2stat(de->li, st);

    st->st_ino = de->inode;
}

void direntry_still_exists (direntry_t *de)
{
    NOT_USED(de);
}

void direntry_no_longer_exists (direntry_t *de)
{
    NOT_USED(de);
}


/* stubs etc ===================================================== */

/* FIXME: stubs */
int direntry_get_by_inode (ino_t ino, direntry_t **de)
{
    direntry_trace("direntry_get_by_inode(ino %lu)\n", ino);

    *de = inode_map_get(ino);


    return *de ? 0 : ENOENT;
}

int direntry_get_child_by_name (
    ino_t parent,
    const char *name,
    direntry_t **de_out
)
{
    direntry_t *de, *child = NULL, *old_child;
    int rc;


    rc = direntry_get_by_inode(parent, &de);

    if (!rc)
    {
        direntry_ensure_children(de);

        /* FIXME: this (will) bypasses cache. Also, probably want to factor to
         * direntry_find_child(de, name).
         * If it's to stay like this, it need to take a lock, as it doesn't own
         * any copies of the children! */
        rc = ENOENT;
        child = direntry_get_first_child(de);
        while (child)
        {
            if (!strcmp(direntry_get_name(child), name))
            {
                rc = 0;
                break;
            }

            old_child = child;
            child = direntry_get_next_sibling(child);
            direntry_delete(CALLER_INFO old_child);
        }

        direntry_delete(CALLER_INFO de);
    }

    if (!rc)
    {
        *de_out = child;
    }


    return rc;
}
