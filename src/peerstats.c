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
    unsigned i;


    assert(alts);


    srandom(time(NULL));


    split_list(alts,
               config_peers_favourites, config_peers_blocked,
               &fav_list, &normal_list, &block_list);


    peerstats_trace("favs count: %u\n", fav_list->count);
    peerstats_trace_indent();
    for (i = 0; i < fav_list->count; ++i)
    {
        peerstats_trace("%s\n", listing_get_client(fav_list->items[i]));
    }
    peerstats_trace_dedent();

    peerstats_trace("normal count: %u\n", normal_list->count);
    peerstats_trace_indent();
    for (i = 0; i < normal_list->count; ++i)
    {
        peerstats_trace("%s\n", listing_get_client(normal_list->items[i]));
    }
    peerstats_trace_dedent();

    peerstats_trace("blocked count: %u\n", block_list->count);
    peerstats_trace_indent();
    for (i = 0; i < block_list->count; ++i)
    {
        peerstats_trace("%s\n", listing_get_client(block_list->items[i]));
    }
    peerstats_trace_dedent();


    if (fav_list->count)
    {
        ret = listing_post(CALLER_INFO fav_list->items[random() % fav_list->count]);
    }
    else if (normal_list->count)
    {
        ret = listing_post(CALLER_INFO normal_list->items[random() % normal_list->count]);
    }
    else
    {
        ret = NULL;
    }

    if (ret)
    {
        peerstats_trace("chose alternative %s from %s\n", listing_get_href(ret), listing_get_client(ret));
    }
    else
    {
        peerstats_trace("no appropriate alternative\n");
    }


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
    listing_list_t *fav_list, *block_list, *normal_list;


    fav_list    = listing_list_new(alts->count);
    block_list  = listing_list_new(alts->count);
    normal_list = listing_list_new(alts->count);


    for (i = 0; i < alts->count; ++i)
    {
        found = 0;

        j = 0;
        while (favs[j])
        {
            if (!strcmp(listing_get_client(alts->items[i]), favs[j]))
            {
                fav_list->items[fav_count++] = listing_post(CALLER_INFO alts->items[i]);
                found = 1;
                break;
            }
            j++;
        }

        if (!found)
        {
            j = 0;
            while (blocks[j])
            {
                if (!strcmp(listing_get_client(alts->items[i]), blocks[j]))
                {
                    block_list->items[block_count++] = listing_post(CALLER_INFO alts->items[i]);
                    found = 1;
                    break;
                }
                j++;
            }
        }

        if (!found)
        {
            normal_list->items[normal_count++] = listing_post(CALLER_INFO alts->items[i]);
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
