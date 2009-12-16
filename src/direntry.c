/*
 * This file deals with "direntries" - "entries in a directory". This is
 * basically our internal file system tree implementation.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
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
#include "direntry_internal.h"
#if FEATURE_DIRENTRY_CACHE
#include "direntry_cache.h"
#endif
#include "fetcher.h"
#include "parser.h"


TRACE_DEFINE(direntry)


static direntry_t *lis_to_dirents(listing_list_t *lis, const char *parent);
static direntry_type_t direntry_type_from_string (const char * const s);
static void listing_to_direntry (listing_t *li, direntry_t *de);


/* direntry module external interface ======================================= */

int direntry_init (void)
{
    direntry_trace("direntry_init()\n");


    return 0;
}

void direntry_finalise (void)
{
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
    direntry_t *de = NULL, *dirents = NULL;
    listing_list_t *lis;
    direntry_cache_status_t cache_stat;


#if FEATURE_DIRENTRY_CACHE
    cache_stat = direntry_cache_get(CALLER_INFO path, &de);

    if (cache_stat == direntry_cache_status_NOENT && config_option_cache_negative)
    {
        rc = -ENOENT;
        goto end;
    }
    else if (cache_stat == direntry_cache_status_HIT)
    {
        rc = 0;
        goto end;
    }

    assert(cache_stat == direntry_cache_status_UNKNOWN);
#else
    /* special case - there is nowhere whence to get metadata for "/"; it has no
     * parent. Instead, we just make it up. */
    if (!strcmp(path, "/"))
    {
        direntry_trace("\"/\" is special\n");

        de = direntry_new_root(CALLER_INFO_ONLY);
        rc = 0;
        goto end;
    }
#endif


    /* Get listing for the children of this path's parent */
    url = make_url("browse", parent_path + 1);
    rc = parser_fetch_listing(url, &lis);
    free(url);


    if (!rc && lis)
    {
#if FEATURE_DIRENTRY_CACHE
        direntry_cache_status_t cache_stat;

        /* This make direntries for the listing, with the side effect that the
         * dirents are encached. */
        dirents = lis_to_dirents(lis, parent_path);

        /* We actually have no interest in the dirents */
        direntry_delete_list(CALLER_INFO dirents);

        /* Our desired direntry will now be in the cache, if it exists */
        cache_stat = direntry_cache_get(CALLER_INFO path, &de);
        if (cache_stat == direntry_cache_status_HIT)
        {
            rc = 0;
        }
        else
        {
            assert(cache_stat == direntry_cache_status_NOENT);
            rc = -ENOENT;
        }
#else
        /* Search parent's children for desired entry */
        rc = -ENOENT;
        for (unsigned i = 0; i < lis->count; ++i)
        {
            if (!strcmp(listing_get_name(lis->items[i]), file_name))
            {
                de = direntry_new(CALLER_INFO_ONLY);

                listing_to_direntry(lis->items[i], de);

                direntry_attribute_add(de, "fs2-path", path);

                rc = 0;
                break;
            }
        }

        listing_list_delete(CALLER_INFO lis);
#endif
    }


end:
    free(parent_path);
    free(file_name);

    *de_out = de;
    return rc;
}

int path_get_children (
    char const * const parent,
    direntry_t **dirents_out
)
{
    int rc = -EIO;
    char *url;
    listing_list_t *lis = NULL;
    direntry_t *de, *dirents = NULL;
    direntry_cache_status_t cache_stat;


    direntry_trace("path_get_children(%s)\n", parent);


#if FEATURE_DIRENTRY_CACHE
    cache_stat = direntry_cache_get(CALLER_INFO parent, &de);
    if (cache_stat == direntry_cache_status_HIT)
    {
        if (direntry_get_looked_for_children(de))
        {
            dirents = direntry_get_first_child(de);
            direntry_post_list(CALLER_INFO dirents);
            direntry_delete(CALLER_INFO de);
            rc = 0;
            goto end;
        }
        direntry_delete(CALLER_INFO de);
    }
#endif


    /* fetch the directory listing from the indexnode */
    url = make_url("browse", parent + 1);
    rc = parser_fetch_listing(url, &lis);
    free(url);

    if (!rc && lis)
    {
        dirents = lis_to_dirents(lis, parent);

        listing_list_delete(CALLER_INFO lis);
    }


end:
    *dirents_out = dirents;
    return rc;
}

static direntry_t *lis_to_dirents(listing_list_t *lis, const char *parent)
{
    direntry_t *de = NULL, *prev = NULL;
    char *path;
    unsigned i;


    assert(lis);

    /* turn array of li's into linked list of de's, adding path info to each one
     * */
    for (i = 0; i < lis->count; ++i)
    {
        de = direntry_new(CALLER_INFO_ONLY);

        listing_to_direntry(lis->items[i], de);

        /* Currently, the indexnode gives us paths for directories, but not
         * files. In addition, the paths is offers up for directories are
         * bollocks, so we ignore them and make our own paths for both */
        path = (char *)malloc(strlen(direntry_get_base_name(de)) + strlen(parent) + 2);

        strcpy(path, parent);
        if (strcmp(parent, "/")) strcat(path, "/"); /* special case: don't append a trailing "/" onto the parent path if it's the root ("/"), because the root is essentially a directory with an empty name, and we store paths normalised. */
        strcat(path, direntry_get_base_name(de));

        direntry_attribute_add(de, "fs2-path", path);

        free(path);


        de->next = prev;
        prev = de;
    }

#if FEATURE_DIRENTRY_CACHE
    direntry_cache_add_list(de, parent);
#endif

    return de;
}

void listing_attribute_add (
    listing_t * const li,
    const char *name,
    const char *value
)
{
    if (!strcmp(name, "fs2-name"))
    {
        li->name = strdup(value);
    }
    else if (!strcmp(name, "fs2-hash"))
    {
        li->hash = strdup(value);
    }
    else if (!strcmp(name, "fs2-type"))
    {
        li->type = direntry_type_from_string(value);
    }
    else if (!strcmp(name, "fs2-size"))
    {
        li->size = atoll(value);
    }
    else if (!strcmp(name, "fs2-linkcount") ||
             !strcmp(name, "fs2-alternativescount"))
    {
        li->link_count = atol(value);
    }
    else if (!strcmp(name, "href"))
    {
        li->href = strdup(value);
    }
    else if (!strcmp(name, "fs2-clientalias"))
    {
        li->client = strdup(value);
    }
    else
    {
        direntry_trace("Unknown attribute %s == %s\n", name, value);
    }
}
void direntry_attribute_add (
    direntry_t * const de,
    const char *name,
    const char *value
)
{
    if (!strcmp(name, "fs2-name"))
    {
        de->base_name = strdup(value);
    }
    else if (!strcmp(name, "fs2-path"))
    {
        de->path = strdup(value);
    }
    else if (!strcmp(name, "fs2-hash"))
    {
        de->hash = strdup(value);
    }
    else if (!strcmp(name, "fs2-type"))
    {
        de->type = direntry_type_from_string(value);
    }
    else if (!strcmp(name, "fs2-size"))
    {
        de->size = atoll(value);
    }
    else if (!strcmp(name, "fs2-linkcount") ||
             !strcmp(name, "fs2-alternativescount"))
    {
        de->link_count = atol(value);
    }
    else if (!strcmp(name, "href"))
    {
        de->href = strdup(value);
    }
    else
    {
        direntry_trace("Unknown attribute %s == %s\n", name, value);
    }
}

static direntry_type_t direntry_type_from_string (const char * const s)
{
    direntry_type_t type;


    if (!strcmp(s, "file"))
    {
        type = direntry_type_FILE;
    }
    else if (!strcmp(s, "directory"))
    {
        type = direntry_type_DIRECTORY;
    }
    else
    {
        assert(0);
    }


    return type;
}


/* direntry lifecycle ======================================================= */
direntry_t *direntry_new (CALLER_DECL_ONLY)
{
    direntry_t *de = (direntry_t *)calloc(1, sizeof(direntry_t));


    de->lock = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(de->lock, NULL);

    de->ref_count = 1;

    direntry_trace("[direntry %p] new (" CALLER_FORMAT ") ref %u\n",
                   de, CALLER_PASS de->ref_count);

    return de;
}

direntry_t *direntry_new_root (CALLER_DECL_ONLY)
{
    direntry_t *root = direntry_new(CALLER_PASS_ONLY);


    direntry_attribute_add(root, "fs2-name", "");
    direntry_attribute_add(root, "fs2-path", "/");
    direntry_attribute_add(root, "fs2-type", "directory");
    /* Set the link count to something non-0. We can't query this from the
     * indexnode. We could work it out every time, but that would be tedious.
     * It turns out to be vitally important that this is non-0 if samba is going
     * to share the filesystem */
    direntry_attribute_add(root, "fs2-linkcount", "1");


    return root;
}

direntry_t *direntry_post (CALLER_DECL direntry_t *de)
{
    char trace_str[1024] = "";
    unsigned refc;


    assert(de->ref_count);

    pthread_mutex_lock(de->lock);
    refc = ++de->ref_count;
    pthread_mutex_unlock(de->lock);

#if DEBUG
    sprintf(trace_str,
            "[direntry %p] post (" CALLER_FORMAT ") ref %u",
            (void *)de, CALLER_PASS de->ref_count);
#if FEATURE_DIRENTRY_CACHE
    {
        char *stat = direntry_cache_status(direntry_get_path(de));
        strcat(trace_str, stat);
        free(stat);
    }
#endif /* FEATURE_DIRENTRY_CACHE */
    strcat(trace_str, "\n");
    direntry_trace(trace_str);
#endif /* DEBUG */


    return de;
}

void direntry_post_list (CALLER_DECL direntry_t *de)
{
    direntry_t *next;

    while (de)
    {
        next = direntry_get_next_sibling(de);
        direntry_post(CALLER_PASS de);
        de = next;
    }
}


void direntry_delete (CALLER_DECL direntry_t *de)
{
    unsigned refc;
    char trace_str[1024];


    /* hacky attempt to detect overflow */
    assert((signed)de->ref_count > 0);

    pthread_mutex_lock(de->lock);
    refc = --de->ref_count;
    pthread_mutex_unlock(de->lock);

#if DEBUG
    sprintf(trace_str,
            "[direntry %p] delete (" CALLER_FORMAT ") ref %u",
            (void *)de, CALLER_PASS de->ref_count);
#if FEATURE_DIRENTRY_CACHE
    {
        char *stat = direntry_cache_status(direntry_get_path(de));
        strcat(trace_str, stat);
        free(stat);
    }
#endif /* FEATURE_DIRENTRY_CACHE */
    strcat(trace_str, "\n");
    direntry_trace(trace_str);
#endif /* DEBUG */

    direntry_trace_indent();

    if (!refc)
    {
        direntry_trace("refcount == 0 => free()ing\n");

        pthread_mutex_destroy(de->lock);
        free(de->lock);

        if (de->base_name) free(de->base_name);
        if (de->path) free(de->path);
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

direntry_t *direntry_set_next_sibling (direntry_t *de, direntry_t *sibling)
{
    return de->next = sibling;
}


/* direntry attribute getters =============================================== */

char *direntry_get_path (direntry_t *de)
{
    return de->path;
}

char *direntry_get_base_name (direntry_t *de)
{
    return de->base_name;
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
    return !strcmp(direntry_get_path(de), "/");
}

int direntry_de2stat (struct stat *st, direntry_t *de)
{
    int uid = config_attr_id_uid,
        gid = config_attr_id_gid;


    memset((void *)st, 0, sizeof(struct stat));

    st->st_uid = (uid == -1) ? getuid() : (unsigned)uid;
    st->st_gid = (gid == -1) ? getgid() : (unsigned)gid;
    st->st_nlink = de->link_count;

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


    return 0;
}

void direntry_still_exists (direntry_t *de)
{
#if FEATURE_DIRENTRY_CACHE
    direntry_cache_notify_still_valid(de);
#else
    NOT_USED(de);
#endif
}

void direntry_no_longer_exists (direntry_t *de)
{
#if FEATURE_DIRENTRY_CACHE
    direntry_cache_notify_stale(de);
#else
    NOT_USED(de);
#endif
}


/* listing lifecycle ======================================================= */
listing_t *listing_new (CALLER_DECL_ONLY)
{
    listing_t *li = (listing_t *)calloc(1, sizeof(listing_t));


    li->lock = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(li->lock, NULL);

    li->ref_count = 1;

    direntry_trace("[listing %p] new (" CALLER_FORMAT ") ref %u\n",
                   li, CALLER_PASS li->ref_count);

    return li;
}

listing_t *listing_post (CALLER_DECL listing_t *li)
{
    unsigned refc;


    assert(li->ref_count);

    pthread_mutex_lock(li->lock);
    refc = ++li->ref_count;
    pthread_mutex_unlock(li->lock);

    direntry_trace("[listing %p] post (" CALLER_FORMAT ") ref %u\n",
                   li, CALLER_PASS refc);


    return li;
}

void listing_delete (CALLER_DECL listing_t *li)
{
    unsigned refc;


    /* hacky attempt to detect overflow */
    assert((signed)li->ref_count > 0);

    pthread_mutex_lock(li->lock);
    refc = --li->ref_count;
    pthread_mutex_unlock(li->lock);

    direntry_trace("[listing %p] delete (" CALLER_FORMAT ") ref %u\n",
                   li, CALLER_PASS refc);
    direntry_trace_indent();

    if (!refc)
    {
        direntry_trace("refcount == 0 => free()ing\n");

        pthread_mutex_destroy(li->lock);
        free(li->lock);

        if (li->name)   free(li->name);
        if (li->hash)   free(li->hash);
        if (li->href)   free(li->href);
        if (li->client) free(li->client);

        free(li);
    }

    direntry_trace_dedent();
}

listing_list_t *listing_list_new (unsigned count)
{
    listing_list_t *lis = (listing_list_t *)malloc(sizeof(listing_list_t));


    lis->count = count;
    lis->items = (listing_t **)calloc(count, sizeof(listing_t *));


    return lis;
}

listing_list_t *listing_list_resize (listing_list_t *lis, unsigned new_count)
{
    lis->items = (listing_t **)realloc(lis->items, new_count * sizeof(listing_t *));

    if (new_count > lis->count)
    {
        memset(lis->items + lis->count, 0, (new_count - lis->count) * sizeof(listing_t *));
    }

    lis->count = new_count;


    return lis;
}


void listing_list_delete (CALLER_DECL listing_list_t *lis)
{
    unsigned i;


    for (i = 0; i < lis->count; ++i)
    {
        listing_delete(CALLER_PASS lis->items[i]);
    }

    free(lis->items);
    free(lis);
}

/* listing attribute getters =============================================== */

char *listing_get_name (listing_t *li)
{
    return li->name;
}

char *listing_get_hash (listing_t *li)
{
    return li->hash;
}

direntry_type_t listing_get_type (listing_t *li)
{
    return li->type;
}

off_t listing_get_size (listing_t *li)
{
    return li->size;
}

unsigned long listing_get_link_count (listing_t *li)
{
    return li->link_count;
}

char *listing_get_href (listing_t *li)
{
    return li->href;
}

char *listing_get_client (listing_t *li)
{
    return li->client;
}

static void listing_to_direntry (listing_t *li, direntry_t *de)
{
    de->type       = li->type;

    switch (li->type)
    {
        case direntry_type_FILE:
            de->base_name  = strdup(li->name);
            de->hash       = strdup(li->hash);
            de->size       = li->size;
            de->link_count = li->link_count;
            de->href       = strdup(li->href);
            break;

        case direntry_type_DIRECTORY:
            de->base_name  = strdup(li->name);
            de->size       = li->size;
            de->link_count = li->link_count;
            de->href       = strdup(li->href);
            break;
    }
}
