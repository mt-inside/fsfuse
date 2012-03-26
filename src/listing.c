/*
 * This file deals with "listings", which are descriptions of all the nodes in a
 * directory.
 *
 * Copyright (C) Matthew Turner 2008-2010. All rights reserved.
 *
 * $Id: direntry.h 519 2010-04-21 21:54:28Z matt $
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "config.h"
#include "listing.h"


TRACE_DEFINE(listing)


struct _listing_t
{
    char                      *name;
    char                      *hash;
    listing_type_t             type;
    off_t                      size; /* st_size in struct stat is off_t */
    unsigned long              link_count;
    char                      *href;

    char                      *client;

    unsigned                   ref_count;
    pthread_mutex_t           *lock;

};

struct _listing_list_t
{
    unsigned count;
    listing_t **items;
};


/* static helpers =========================================================== */

static listing_type_t listing_type_from_string (const char * const s)
{
    listing_type_t type;


    if (!strcmp(s, "file"))
    {
        type = listing_type_FILE;
    }
    else if (!strcmp(s, "directory"))
    {
        type = listing_type_DIRECTORY;
    }
    else
    {
        assert(0);
    }


    return type;
}


/* listing lifecycle ======================================================= */

listing_t *listing_new (CALLER_DECL_ONLY)
{
    listing_t *li = (listing_t *)calloc(1, sizeof(listing_t));


    li->lock = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(li->lock, NULL);

    li->ref_count = 1;

    listing_trace("[listing %p] new (" CALLER_FORMAT ") ref %u\n",
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

    listing_trace("[listing %p] post (" CALLER_FORMAT ") ref %u\n",
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

    listing_trace("[listing %p] delete (" CALLER_FORMAT ") ref %u\n",
                   li, CALLER_PASS refc);
    listing_trace_indent();

    if (!refc)
    {
        listing_trace("refcount == 0 => free()ing\n");

        pthread_mutex_destroy(li->lock);
        free(li->lock);

        if (li->name)   free(li->name);
        if (li->hash)   free(li->hash);
        if (li->href)   free(li->href);
        if (li->client) free(li->client);

        free(li);
    }

    listing_trace_dedent();
}


/* listing equality ========================================================= */

int listing_equal (listing_t *li, listing_t *other)
{
    return hash_equal(li->hash, other->hash);
}


/* listing attribute getters ================================================ */

char *listing_get_name (listing_t *li)
{
    return li->name;
}

char *listing_get_hash (listing_t *li)
{
    return li->hash;
}

listing_type_t listing_get_type (listing_t *li)
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

void listing_li2stat (listing_t *li, struct stat *st)
{
    int uid = config_attr_id_uid,
        gid = config_attr_id_gid;


    memset((void *)st, 0, sizeof(struct stat));

    st->st_nlink = li->link_count;
    st->st_uid = (uid == -1) ? getuid() : (unsigned)uid;
    st->st_gid = (gid == -1) ? getgid() : (unsigned)gid;

    switch (li->type)
    {
        /* Regular file */
        case listing_type_FILE:
            st->st_mode = S_IFREG | config_attr_mode_file;

            st->st_size = li->size;
            st->st_blksize = FSFUSE_BLKSIZE;
            st->st_blocks = (li->size / 512) + 1;

            break;
        case listing_type_DIRECTORY:
            st->st_mode = S_IFDIR | config_attr_mode_dir;

            /* indexnode supplies directory's tree size - not what a unix fs
             * wants */
            st->st_size = 0;

            break;
    }
}


/* listing list lifecycle =================================================== */

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


/* listing list getters ===================================================== */

unsigned listing_list_get_count (listing_list_t *lis)
{
    return lis->count;
}

void listing_list_set_item (listing_list_t *lis, unsigned item, listing_t *li)
{
    lis->items[item] = listing_post(CALLER_INFO li);
}

listing_t *listing_list_get_item (listing_list_t *lis, unsigned item)
{
    return listing_post(CALLER_INFO lis->items[item]);
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
        li->type = listing_type_from_string(value);
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
        listing_trace("Unknown attribute %s == %s\n", name, value);
    }
}
