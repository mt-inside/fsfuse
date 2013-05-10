/*
 * Copyright (C) 2008-2013 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Listing list class implementation.
 */

#include "common.h"

#include <stdlib.h>
#include <string.h>

#include "listing_list.h"


struct _listing_list_t
{
    unsigned count;
    listing_t **items;
};


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
    lis->items[item] = listing_copy(CALLER_INFO li);
}

listing_t *listing_list_get_item (listing_list_t *lis, unsigned item)
{
    return listing_copy(CALLER_INFO lis->items[item]);
}
