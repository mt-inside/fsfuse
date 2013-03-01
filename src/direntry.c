/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * This file deals with "direntries" - "entries in a directory". This is
 * basically our internal file system tree implementation.
 */

#include "common.h"

#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "direntry.h"

#include "config.h"
#include "fetcher.h"
#include "indexnodes.h"
#include "inode_map.h"
#include "parser.h"
#include "ref_count.h"
#include "string_buffer.h"


TRACE_DEFINE(direntry)


/* TODO:
 * wtf does this add ref_counting fields again?
 * wtf dies this have-an li, when it could /be/ and li?
 */
struct _direntry_t
{
    listing_t          *li;

    ref_count_t        *ref_count;

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
    const char *path, *url;
    listing_list_t *lis = NULL;
    direntry_t *dirents = NULL;


    if (!de->looked_for_children)
    {
        path = direntry_get_path(de);
        direntry_trace("direntry_get_children(%s)\n", path);


        /* fetch the directory listing from the indexnode */
        url = direntry_make_url(de, "browse", path + 1);
        rc = parser_fetch_listing(url, &lis);
        free_const(path);
        free_const(url);

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


    de->ref_count = ref_count_new( );

    de->inode = inode;
    inode_map_add(de);

    direntry_trace(
        "[direntry %p inode %lu] new (" CALLER_FORMAT ") ref %u\n",
        de, de->inode, CALLER_PASS 1
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
    unsigned refc = ref_count_inc( de->ref_count );

    direntry_trace("[direntry %p inode %lu] post (" CALLER_FORMAT ") ref %u\n",
                   de, de->inode, CALLER_PASS refc);


    return de;
}

void direntry_delete (CALLER_DECL direntry_t *de)
{
    unsigned refc = ref_count_dec( de->ref_count );

    direntry_trace("[direntry %p inode %lu] delete (" CALLER_FORMAT ") ref %u\n",
                   de, de->inode, CALLER_PASS refc);
    direntry_trace_indent();

    if (!refc)
    {
        direntry_trace("refcount == 0 => free()ing\n");

        ref_count_delete( de->ref_count );

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

listing_t *direntry_peek_listing (direntry_t *de)
{
    return de->li;
}

ino_t direntry_get_inode (direntry_t *de)
{
    return de->inode;
}

static void direntry_get_path_inner (direntry_t *de, string_buffer_t *path)
{
    direntry_t *parent;


    if ((parent = direntry_get_parent(de)))
    {
        direntry_get_path_inner(parent, path);
        direntry_delete(CALLER_INFO parent);
    }

    string_buffer_append(path, direntry_get_name(de));
    string_buffer_append(path, "/");
}
char *direntry_get_path (direntry_t *de)
{
    string_buffer_t *path = string_buffer_new();


    direntry_get_path_inner(de, path);


    return string_buffer_commit(path);
}

char *direntry_get_name (direntry_t *de)
{
    return listing_get_name(de->li);
}

char *direntry_get_hash (direntry_t *de)
{
    return listing_get_hash(de->li);
}

listing_type_t direntry_get_type (direntry_t *de)
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

const char *direntry_make_url (
    direntry_t *de,
    const char * const path_prefix,
    const char * const resource
)
{
    return listing_make_url(de->li, path_prefix, resource);
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
