/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * This file deals with "listings", which are descriptions of all the nodes in a
 * directory.
 */

#include "common.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "listing.h"
#include "listing_internal.h"

#include "config_manager.h"
#include "config_reader.h"
#include "fs2_constants.h"
#include "indexnode.h"
#include "parser.h"
#include "ref_count.h"
#include "utils.h"


TRACE_DEFINE(listing)


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

listing_t *listing_new (
    CALLER_DECL
    indexnode_t *in,
    const char *hash,
    const char *name,
    const char *type,
    off_t size,
    unsigned long link_count,
    const char *href,
    const char *client
)
{
    listing_t *li = malloc(sizeof(listing_t));


    li->ref_count = ref_count_new();

    li->in = in;
    li->name = name;
    li->hash = hash;
    li->type = listing_type_from_string(type);
    li->size = size;
    li->link_count = link_count;
    li->href = href;
    li->client = client;

    listing_trace("[listing %p] new (" CALLER_FORMAT ") ref %u\n",
                   li, CALLER_PASS 1);


    return li;
}

listing_t *listing_post (CALLER_DECL listing_t *li)
{
    unsigned refc = ref_count_inc( li->ref_count );


    NOT_USED(refc);
    listing_trace("[listing %p] post (" CALLER_FORMAT ") ref %u\n",
                   li, CALLER_PASS refc);


    return li;
}

void listing_teardown (listing_t *li)
{
    ref_count_delete( li->ref_count );

    /* TODO: be explicti about whic of these are mandatory (e.g. name) and
     * assert on the way in and don't check here */
    if (li->name)   free_const(li->name);
    if (li->hash)   free_const(li->hash);
    if (li->href)   free_const(li->href);
    if (li->client) free_const(li->client);
}

void listing_delete (CALLER_DECL listing_t *li)
{
    unsigned refc = ref_count_dec( li->ref_count );

    listing_trace("[listing %p] delete (" CALLER_FORMAT ") ref %u\n",
                   li, CALLER_PASS refc);
    listing_trace_indent();

    if (!refc)
    {
        listing_trace("refcount == 0 => free()ing\n");

        listing_teardown(li);
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
    return strdup( li->name );
}

char *listing_get_hash (listing_t *li)
{
    return strdup( li->hash );
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
    return strdup( li->href );
}

char *listing_get_client (listing_t *li)
{
    return strdup( li->client );
}

void listing_li2stat (listing_t *li, struct stat *st)
{
    config_reader_t *config = config_get_reader();
    int uid = config_attr_id_uid(config),
        gid = config_attr_id_gid(config);


    memset((void *)st, 0, sizeof(struct stat));

    st->st_nlink = li->link_count;
    st->st_uid = (uid == -1) ? getuid() : (unsigned)uid;
    st->st_gid = (gid == -1) ? getgid() : (unsigned)gid;

    switch (li->type)
    {
        /* Regular file */
        case listing_type_FILE:
            st->st_mode = S_IFREG | config_attr_mode_file(config);

            st->st_size = li->size;
            st->st_blksize = FSFUSE_BLKSIZE;
            st->st_blocks = (li->size / 512) + 1;

            break;
        case listing_type_DIRECTORY:
            st->st_mode = S_IFDIR | config_attr_mode_dir(config);

            /* indexnode supplies directory's tree size - not what a unix fs
             * wants */
            st->st_size = 0;

            break;
    }
}

int listing_tryget_best_alternative( listing_t *li_reference, listing_t **li_best )
{
    return indexnode_tryget_best_alternative(
        indexnode_post( CALLER_INFO li_reference->in ),
        listing_get_hash( li_reference ),
        li_best
    );
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
