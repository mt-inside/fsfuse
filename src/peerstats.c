/*
 * Peer Statistics Module
 *
 * Copyright (C) Matthew Turner 2009. All rights reserved.
 *
 * $Id: hash.c 366 2009-10-23 14:09:30Z matt $
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
#endif
    unsigned i;


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
        found = 0;

        j = 0;
        while (favs[j])
        {
            li = listing_list_get_item(alts, i);
            if (!strcmp(listing_get_client(li), favs[j]))
            {
                listing_list_set_item(fav_list, fav_count++, li);
                found = 1;
                break;
            }
            listing_delete(CALLER_INFO li);
            j++;
        }

        if (!found)
        {
            j = 0;
            while (blocks[j])
            {
                li = listing_list_get_item(alts, i);
                if (!strcmp(listing_get_client(li), blocks[j]))
                {
                    listing_list_set_item(block_list, block_count++, li);
                    found = 1;
                    break;
                }
                listing_delete(CALLER_INFO li);
                j++;
            }
        }

        if (!found)
        {
            li = listing_list_get_item(alts, i);
            listing_list_set_item(normal_list, normal_count++, li);
            listing_delete(CALLER_INFO li);
        }
    }

    assert(fav_count + block_count + normal_count == i);

    listing_list_resize(fav_list,    fav_count   );
    listing_list_resize(block_list,  block_count );
    listing_list_resize(normal_list, normal_count);


    *fav_list_out = fav_list;
    *block_list_out = block_list;
    *normal_list_out = normal_list;
}
