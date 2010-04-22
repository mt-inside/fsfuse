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
#include <pthread.h>

#include "common.h"
#include "config.h"
#include "direntry.h"
#include "fetcher.h"
#include "inode_map.h"
#include "parser.h"


TRACE_DEFINE(direntry)


struct _direntry_t
{
    char                      *name;
    char                      *hash;
    direntry_type_t            type;
    off_t                      size; /* st_size in struct stat is off_t */
    unsigned long              link_count;
    char                      *href;

    unsigned                   ref_count;
    pthread_mutex_t           *lock;

    ino_t                      inode;
    struct _direntry_t        *next;
    struct _direntry_t        *parent;
    struct _direntry_t        *children;
    int                        looked_for_children;
};


static direntry_t *direntry_new (CALLER_DECL_ONLY);
extern direntry_t *direntry_new_root (CALLER_DECL_ONLY);
static direntry_t *direntry_from_listing (listing_t *li);
static direntry_t *direntries_from_listing_list (listing_list_t *lis, direntry_t *parent);


/* direntry module external interface ======================================= */

int direntry_init (void)
{
    direntry_trace("direntry_init()\n");


    /* TODO: horrid way to encache */
    direntry_delete(CALLER_INFO direntry_new_root(CALLER_INFO_ONLY));


    return 0;
}

void direntry_finalise (void)
{
    inode_map_clear();
}


/* Innards ================================================================== */

int path_get_direntry (
    char const * const path,
    direntry_t **de_out
)
{
    int rc;
    char *parent_path = fsfuse_dirname(path),
         *file_name   = fsfuse_basename(path),
         *url;
    direntry_t *de = NULL;
    listing_list_t *lis;
    listing_t *li;


    /* special case - there is nowhere whence to get metadata for "/"; it has no
     * parent. Instead, we just make it up. */
    if (!strcmp(path, "/"))
    {
        direntry_trace("\"/\" is special\n");

        de = direntry_new_root(CALLER_INFO_ONLY);
        rc = 0;
        goto end;
    }


    /* Get listing for the children of this path's parent */
    url = make_url("browse", parent_path + 1);
    rc = parser_fetch_listing(url, &lis);
    free(url);


    if (!rc && lis)
    {
        /* Search parent's children for desired entry */
        rc = ENOENT;
        for (unsigned i = 0; i < listing_list_get_count(lis); ++i)
        {
            li = listing_list_get_item(lis, i);
            if (!strcmp(listing_get_name(li), file_name))
            {
                /* FIXME!! should this not store return in de??? */
                direntry_from_listing(li);

                rc = 0;
                break;
            }
            listing_delete(CALLER_INFO li);
        }
        listing_list_delete(CALLER_INFO lis);
    }


end:
    free(parent_path);
    free(file_name);

    *de_out = de;
    return rc;
}

int direntry_get_children (
    direntry_t *de,
    direntry_t **children_out
)
{
    int rc = EIO;
    char *path, *url;
    listing_list_t *lis = NULL;
    direntry_t *dirents = NULL;


    if (de->children)
    {
        *children_out = direntry_post_list(CALLER_INFO de->children);
        rc = 0;
    }
    else
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
        *children_out = dirents;
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
        de = direntry_from_listing(li);
        listing_delete(CALLER_INFO li);


        de->parent = parent;
        de->next = prev;
        prev = de;
    }


    return de;
}

static direntry_t *direntry_from_listing (listing_t *li)
{
    direntry_t *de = direntry_new(CALLER_INFO_ONLY);


    de->type       = listing_get_type(li);

    switch (de->type)
    {
        case direntry_type_FILE:
            de->name       = strdup(listing_get_name(li));
            de->hash       = strdup(listing_get_hash(li));
            de->size       = listing_get_size(li);
            de->link_count = listing_get_link_count(li);
            de->href       = strdup(listing_get_href(li));
            break;

        case direntry_type_DIRECTORY:
            de->name       = strdup(listing_get_name(li));
            de->size       = listing_get_size(li);
            de->link_count = listing_get_link_count(li);
            de->href       = strdup(listing_get_href(li));
            break;
    }


    return de;
}


/* direntry lifecycle ======================================================= */

static direntry_t *direntry_new (CALLER_DECL_ONLY)
{
    direntry_t *de = (direntry_t *)calloc(1, sizeof(direntry_t));


    de->lock = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(de->lock, NULL);

    de->ref_count = 1;
    de->inode = inode_next();
    inode_map_add(de);

    direntry_trace("[direntry %p] new (" CALLER_FORMAT ") ref %u inode %lu\n",
                   de, CALLER_PASS de->ref_count, de->inode);

    return de;
}

direntry_t *direntry_new_root (CALLER_DECL_ONLY)
{
    direntry_t *root = (direntry_t *)calloc(1, sizeof(direntry_t));


    root->lock = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(root->lock, NULL);

    root->ref_count = 1;
    root->name = strdup("");
    root->type = direntry_type_DIRECTORY;
    /* Set the link count to something non-0. We can't query this from the
     * indexnode. We could work it out every time, but that would be tedious.
     * It turns out to be vitally important that this is non-0 if samba is going
     * to share the filesystem */
    root->link_count = 1;

    root->inode = 1;
    inode_map_add(root);

    direntry_trace("[direntry %p] new (" CALLER_FORMAT ") ref %u inode %lu\n",
                   root, CALLER_PASS root->ref_count, root->inode);

    return root;
}

direntry_t *direntry_post (CALLER_DECL direntry_t *de)
{
#if DEBUG
    char trace_str[1024] = "";
#endif
    unsigned refc;


    assert(de->ref_count);

    pthread_mutex_lock(de->lock);
    refc = ++de->ref_count;
    pthread_mutex_unlock(de->lock);

#if DEBUG
    sprintf(trace_str,
            "[direntry %p] post (" CALLER_FORMAT ") ref %u",
            (void *)de, CALLER_PASS de->ref_count);
    strcat(trace_str, "\n");
    direntry_trace(trace_str);
#endif /* DEBUG */


    return de;
}

direntry_t *direntry_post_list (CALLER_DECL direntry_t *de)
{
    direntry_t *next = de;


    while (next)
    {
        next = direntry_get_next_sibling(direntry_post(CALLER_PASS next));
    }


    return de;
}


void direntry_delete (CALLER_DECL direntry_t *de)
{
    unsigned refc;
#if DEBUG
    char trace_str[1024];
#endif


#if DEBUG
    sprintf(trace_str,
            "[direntry %p] delete (" CALLER_FORMAT ") ref %u",
            (void *)de, CALLER_PASS de->ref_count - 1);
    strcat(trace_str, "\n");
    direntry_trace(trace_str);
#endif /* DEBUG */

    /* hacky attempt to detect overflow */
    assert((signed)de->ref_count > 0);

    pthread_mutex_lock(de->lock);
    refc = --de->ref_count;
    pthread_mutex_unlock(de->lock);

    direntry_trace_indent();

    if (!refc)
    {
        direntry_trace("refcount == 0 => free()ing\n");

        pthread_mutex_destroy(de->lock);
        free(de->lock);

        if (de->name) free(de->name);
        if (de->hash) free(de->hash);
        if (de->href) free(de->href);

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


/* direntry tree traversal functions ======================================== */
/* for now, we only provide these simple wrappers around the underlying
 * implementation details. Iterator functions could be built from these */

direntry_t *direntry_get_parent       (direntry_t *de)
{
    return de->parent;
}

direntry_t *direntry_get_first_child  (direntry_t *de)
{
    return de->children;
}

direntry_t *direntry_get_next_sibling (direntry_t *de)
{
    return de->next;
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
    }

    strcat(path, direntry_get_base_name(de));
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

char *direntry_get_base_name (direntry_t *de)
{
    return de->name;
}

char *direntry_get_hash (direntry_t *de)
{
    return de->hash;
}

direntry_type_t direntry_get_type (direntry_t *de)
{
    return de->type;
}

off_t direntry_get_size (direntry_t *de)
{
    return de->size;
}

unsigned long direntry_get_link_count (direntry_t *de)
{
    return de->link_count;
}

char *direntry_get_href (direntry_t *de)
{
    return de->href;
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
    char *path = direntry_get_path(de);
    int is_root = !strcmp(direntry_get_path(de), "/");

    free(path);

    return is_root;
}

void direntry_de2stat (direntry_t *de, struct stat *st)
{
    int uid = config_attr_id_uid,
        gid = config_attr_id_gid;


    memset((void *)st, 0, sizeof(struct stat));

    st->st_ino = de->inode;
    st->st_nlink = de->link_count;
    st->st_uid = (uid == -1) ? getuid() : (unsigned)uid;
    st->st_gid = (gid == -1) ? getgid() : (unsigned)gid;

    switch (de->type)
    {
        /* Regular file */
        case direntry_type_FILE:
            st->st_mode = S_IFREG | config_attr_mode_file;

            st->st_size = de->size;
            st->st_blksize = FSFUSE_BLKSIZE;
            st->st_blocks = (de->size / 512) + 1;

            break;
        case direntry_type_DIRECTORY:
            st->st_mode = S_IFDIR | config_attr_mode_dir;

            /* indexnode supplies directory's tree size - not what a unix fs
             * wants */
            st->st_size = 0;

            break;
    }
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
int direntry_get_by_inode (fuse_ino_t ino, direntry_t **de)
{
    direntry_trace("direntry_get_by_inode(ino %lu)\n", ino);

    *de = inode_map_get(ino);


    return *de ? 0 : ENOENT;
}

int direntry_get_child_by_name (
    fuse_ino_t parent,
    const char *name,
    direntry_t **de_out
)
{
    direntry_t *de, *first_child, *child = NULL;
    int rc;


    rc = direntry_get_by_inode(parent, &de);
    /* ensure that the children have been fetched. TODO split this into
     * "ensure/fetch_children" and "get_children". */
    rc = direntry_get_children(de, &first_child);
    direntry_delete_list(CALLER_INFO first_child);

    if (!rc)
    {
        /* FIXME: this (will) bypasses cache. Also, probably want to factor to
         * direntry_find_child(de, name).
         * If it's to stay like this, it need to take a lock, as it doesn't own
         * any copies of the children! */
        rc = ENOENT;
        child = de->children;
        while (child)
        {
            if (!strcmp(direntry_get_base_name(child), name))
            {
                rc = 0;
                direntry_post(CALLER_INFO child);
                break;
            }

            child = direntry_get_next_sibling(child);
        }

        direntry_delete(CALLER_INFO de);
    }

    if (!rc)
    {
        *de_out = child;
    }


    return rc;
}
