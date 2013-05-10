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
#include "listing_internal.h"
#include "listing_list.h"

#include "fetcher.h"
#include "fs2_constants.h"
#include "inode_map.h"
#include "ref_count.h"
#include "string_buffer.h"


TRACE_DEFINE(direntry)


struct _direntry_t
{
    listing_t           li;

    ino_t               inode;
    struct _direntry_t *next;
    struct _direntry_t *parent;
    struct _direntry_t *children;
    int                 looked_for_children;
};


static direntry_t *direntry_new_root (CALLER_DECL_ONLY);
static direntry_t *direntry_from_listing (CALLER_DECL listing_t *li);
static direntry_t *direntries_from_listing_list (listing_list_t *lis, direntry_t *parent);


#define BASE_CLASS(de) ((listing_t *)de)


/* direntry module external interface ======================================= */

/* TODO: wtf. At startup just call get_root_direntry or somethign */
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

static void direntry_get_path_inner (direntry_t *de, string_buffer_t *path)
{
    direntry_t *parent;


    if ((parent = direntry_get_parent(de)))
    {
        direntry_get_path_inner(parent, path);
        direntry_delete(CALLER_INFO parent);
    }

    string_buffer_append(path, direntry_get_name(de));
    string_buffer_append(path, strdup("/"));
}
static char *direntry_get_path (direntry_t *de)
{
    string_buffer_t *path = string_buffer_new();


    direntry_get_path_inner(de, path);


    return string_buffer_commit(path);
}


typedef struct
{
    indexnode_t *in;
    listing_list_t *lis;
    unsigned i;
} entry_found_ctxt_t;

static void entry_found(
    void *ctxt_void,
    const char *hash,
    const char *name,
    const char *type,
    off_t size,
    unsigned long link_count,
    const char *href,
    const char *client
)
{
    entry_found_ctxt_t *ctxt = (entry_found_ctxt_t *)ctxt_void;
    listing_t *li;


    li = listing_new( CALLER_INFO indexnode_copy( CALLER_INFO ctxt->in ), hash, name, type, size, link_count, href, client );
    listing_list_resize( ctxt->lis, ctxt->i + 1 ); //FIXME!
    listing_list_set_item( ctxt->lis, ctxt->i++, li );
}

int direntry_ensure_children (
    direntry_t *de
)
{
    int rc = EIO;
    const char *path;
    direntry_t *dirents = NULL;
    entry_found_ctxt_t *ctxt;


    if (!de->looked_for_children)
    {
        path = direntry_get_path(de);
        direntry_trace("direntry_get_children(%s)\n", path);

        ctxt = malloc( sizeof(*ctxt) );
        ctxt->in = BASE_CLASS(de)->in;
        ctxt->lis = listing_list_new( 0 );
        ctxt->i = 0;

        /* skip the leading '/' from the path that fuse gives us */
        if (indexnode_tryget_listing(indexnode_copy(CALLER_INFO BASE_CLASS(de)->in), path + 1, &entry_found, ctxt))
        {
            dirents = direntries_from_listing_list (ctxt->lis, de);
            rc = 0;

            listing_list_delete(CALLER_INFO ctxt->lis);
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

static direntry_t *direntry_from_listing (CALLER_DECL listing_t *li)
{
    /* TODO: stop being lazy and have listing_init */
    direntry_t *de = realloc(li, sizeof(direntry_t));


    bzero(de + sizeof(listing_t), sizeof(direntry_t) - sizeof(listing_t));

    de->inode = inode_next();
    inode_map_add(de);

    direntry_trace(
        "[direntry %p inode %lu] new (" CALLER_FORMAT ") ref %u\n",
        de, de->inode, CALLER_PASS 1
    );


    return de;
}

direntry_t *direntry_new_root (CALLER_DECL_ONLY)
{
    direntry_t *de = (direntry_t *)calloc(1, sizeof(direntry_t));


    /* Set the link count to something non-0. We can't query this from the
     * indexnode. We could work it out every time, but that would be tedious.
     * It turns out to be vitally important that this is non-0 if samba is going
     * to share the filesystem */
    //TODO: calculate me in the future, or get best guess (i.e. the cache might
    //know, else 1)
    //FIXME: this is going to break. How do you list this direntry with a null
    //indexnode? There must be special-case code.
    BASE_CLASS(de)->name = strdup( "" );
    BASE_CLASS(de)->type = listing_type_DIRECTORY;
    BASE_CLASS(de)->link_count = 1;

    de->inode = FSFUSE_ROOT_INODE;
    inode_map_add(de);

    direntry_trace(
        "[direntry %p inode %lu] new (" CALLER_FORMAT ") ref %u\n",
        de, de->inode, CALLER_PASS 1
    );


    return de;
}

direntry_t *direntry_copy (CALLER_DECL direntry_t *de)
{
    unsigned refc = ref_count_inc( BASE_CLASS(de)->ref_count );


    NOT_USED(refc);
    direntry_trace("[direntry %p inode %lu] copy (" CALLER_FORMAT ") ref %u\n",
                   de, de->inode, CALLER_PASS refc);


    return de;
}

void direntry_delete (CALLER_DECL direntry_t *de)
{
    unsigned refc = ref_count_dec( BASE_CLASS(de)->ref_count );

    direntry_trace("[direntry %p inode %lu] delete (" CALLER_FORMAT ") ref %u\n",
                   de, de->inode, CALLER_PASS refc);
    direntry_trace_indent();

    if (!refc)
    {
        direntry_trace("refcount == 0 => free()ing\n");

        listing_teardown(BASE_CLASS(de));

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
    return listing_equal(BASE_CLASS(de), BASE_CLASS(other));
}


/* direntry tree traversal functions ======================================== */
/* for now, we only provide these simple wrappers around the underlying
 * implementation details. Iterator functions could be built from these */

direntry_t *direntry_get_parent       (direntry_t *de)
{
    return de->parent ? direntry_copy(CALLER_INFO de->parent) : NULL;
}

direntry_t *direntry_get_first_child  (direntry_t *de)
{
    return de->children ? direntry_copy(CALLER_INFO de->children) : NULL;
}

direntry_t *direntry_get_next_sibling (direntry_t *de)
{
    return de->next ? direntry_copy(CALLER_INFO de->next) : NULL;
}


/* direntry attribute getters =============================================== */

ino_t direntry_get_inode (direntry_t *de)
{
    return de->inode;
}

char *direntry_get_name (direntry_t *de)
{
    return listing_get_name(BASE_CLASS(de));
}

char *direntry_get_hash (direntry_t *de)
{
    return listing_get_hash(BASE_CLASS(de));
}

listing_type_t direntry_get_type (direntry_t *de)
{
    return listing_get_type(BASE_CLASS(de));
}

off_t direntry_get_size (direntry_t *de)
{
    return listing_get_size(BASE_CLASS(de));
}

unsigned long direntry_get_link_count (direntry_t *de)
{
    return listing_get_link_count(BASE_CLASS(de));
}

char *direntry_get_href (direntry_t *de)
{
    return listing_get_href(BASE_CLASS(de));
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
    listing_li2stat(BASE_CLASS(de), st);

    st->st_ino = de->inode;
}

/* TODO: bit nasty that fuse_entry_param type leaks into here, but then so does
 * struct stat */
void direntry_de2fuse_entry (direntry_t *de, struct fuse_entry_param *entry)
{
    entry->ino = de->inode;;
    entry->generation = 0;
    direntry_de2stat(de, &(entry->attr));
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
