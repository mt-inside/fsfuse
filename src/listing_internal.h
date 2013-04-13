/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * Declarations internal to listing and its friends.
 */

#ifndef _INCLUDED_LISTING_INTERNAL_H
#define _INCLUDED_LISTING_INTERNAL_H

#include "common.h"

#include <sys/types.h>

#include "indexnode.h"
#include "ref_count.h"


struct _listing_t
{
    ref_count_t               *ref_count;

    indexnode_t               *in;
    const char                *name;
    const char                *hash;
    listing_type_t             type;
    off_t                      size; /* st_size in struct stat is off_t */
    unsigned long              link_count;
    const char                *href;

    const char                *client;
};


void listing_teardown (listing_t *li);

#endif /* _INCLUDED_LISTING_INTERNAL_H */
