/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Peer Statistics Module
 */

#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "common.h"
#include "config.h"
#include "peerstats.h"
#include "direntry.h"


TRACE_DEFINE(peerstats)


static void split_list (
    listing_list_t *alts,
    char **favs,
    char **blocks,
    listing_list_t **fav_list_out,
    listing_list_t **normal_list_out,
    listing_list_t **block_list_out
);


int peerstats_init (void)
{
    /* do nothing */

    return 0;
}

void peerstats_finalise (void)
{
    /* do nothing */
}


listing_t *peerstats_chose_alternative (listing_list_t *alts)
{
    listing_list_t *fav_list, *normal_list, *block_list;
    listing_t *ret;
#if DEBUG
    listing_t *li;
    unsigned i;
#endif


    assert(alts);


    srandom(time(NULL));


    split_list(alts,
               config_peers_favourites, config_peers_blocked,
               &fav_list, &normal_list, &block_list);


#if DEBUG
    peerstats_trace("favs count: %u\n", listing_list_get_count(fav_list));
    peerstats_trace_indent();
    for (i = 0; i < listing_list_get_count(fav_list); ++i)
    {
        li = listing_list_get_item(fav_list, i);
        peerstats_trace("%s\n", listing_get_client(li));
        listing_delete(CALLER_INFO li);
    }
    peerstats_trace_dedent();

    peerstats_trace("normal count: %u\n", listing_list_get_count(normal_list));
    peerstats_trace_indent();
    for (i = 0; i < listing_list_get_count(normal_list); ++i)
    {
        li = listing_list_get_item(normal_list, i);
        peerstats_trace("%s\n", listing_get_client(li));
        listing_delete(CALLER_INFO li);
    }
    peerstats_trace_dedent();

    peerstats_trace("blocked count: %u\n", listing_list_get_count(block_list));
    peerstats_trace_indent();
    for (i = 0; i < listing_list_get_count(block_list); ++i)
    {
        li = listing_list_get_item(block_list, i);
        peerstats_trace("%s\n", listing_get_client(li));
        listing_delete(CALLER_INFO li);
    }
    peerstats_trace_dedent();
#endif /* DEBUG */


    if (listing_list_get_count(fav_list))
    {
        ret = listing_list_get_item(fav_list, random() % listing_list_get_count(fav_list));
    }
    else if (listing_list_get_count(normal_list))
    {
        ret = listing_list_get_item(normal_list, random() % listing_list_get_count(normal_list));
    }
    else
    {
        ret = NULL;
    }

#if DEBUG
    if (ret)
    {
        peerstats_trace("chose alternative %s from %s\n", listing_get_href(ret), listing_get_client(ret));
    }
    else
    {
        peerstats_trace("no appropriate alternative\n");
    }
#endif


    listing_list_delete(CALLER_INFO fav_list   );
    listing_list_delete(CALLER_INFO normal_list);
    listing_list_delete(CALLER_INFO block_list );


    return ret;
}


/* TODO: this looks a bit O(n^2) */
static void split_list (
    listing_list_t *alts,
    char **favs,
    char **blocks,
    listing_list_t **fav_list_out,
    listing_list_t **normal_list_out,
    listing_list_t **block_list_out
)
{
    unsigned i = 0, j = 0, fav_count = 0, block_count = 0, normal_count = 0;
    int found;
    listing_t *li;
    listing_list_t *fav_list, *block_list, *normal_list;


    fav_list    = listing_list_new(listing_list_get_count(alts));
    block_list  = listing_list_new(listing_list_get_count(alts));
    normal_list = listing_list_new(listing_list_get_count(alts));


    for (i = 0; i < listing_list_get_count(alts); ++i)
    {
        li = listing_list_get_item(alts, i);
        found = 0;

        if (favs)
        {
            j = 0;
            while (favs[j])
            {
                if (!strcmp(listing_get_client(li), favs[j]))
                {
                    listing_list_set_item(fav_list, fav_count++, li);
                    found = 1;
                    break;
                }
                j++;
            }
        }

        if (!found)
        {
            if (blocks)
            {
                j = 0;
                while (blocks[j])
                {
                    if (!strcmp(listing_get_client(li), blocks[j]))
                    {
                        listing_list_set_item(block_list, block_count++, li);
                        found = 1;
                        break;
                    }
                    j++;
                }
            }
        }

        if (!found)
        {
            listing_list_set_item(normal_list, normal_count++, li);
        }

        listing_delete(CALLER_INFO li);
    }

    assert(fav_count + block_count + normal_count == i);

    listing_list_resize(fav_list,    fav_count   );
    listing_list_resize(block_list,  block_count );
    listing_list_resize(normal_list, normal_count);


    *fav_list_out = fav_list;
    *block_list_out = block_list;
    *normal_list_out = normal_list;
}
