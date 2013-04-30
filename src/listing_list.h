/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * Listing list class.
 * A list of listings.
 */

#ifndef _INCLUDED_LISTING_LIST_H
#define _INCLUDED_LISTING_LIST_H

#include "common.h"

#include "listing.h"


typedef struct _listing_list_t listing_list_t;

extern listing_list_t *listing_list_new (unsigned count);

extern listing_list_t *listing_list_resize (listing_list_t *lis, unsigned new_count);
extern void listing_list_delete (CALLER_DECL listing_list_t *lis);

/* TODO: why does this look like a list, not an enumerable? I.e. why does it
 * have indexes? Probably want to dedup with indexnode_list or something */
extern unsigned listing_list_get_count (listing_list_t *lis);
extern void listing_list_set_item (listing_list_t *lis, unsigned item, listing_t *li);
extern listing_t *listing_list_get_item (listing_list_t *lis, unsigned item);

#endif /* _INCLUDED_LISTING_LIST_H */
