/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * The inode -> direntry map code.
 */

#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "direntry.h"


static direntry_t **s_inode_map = NULL;
static size_t s_inode_map_size = 0;
static size_t s_inode_map_used = 0;


static ino_t s_inode_next = 2;

static const size_t INODE_MAP_MOD = 32;


void inode_map_add (ino_t inode, direntry_t *de)
{
    assert(de);

    if (inode >= s_inode_map_size)
    {
        s_inode_map_size += INODE_MAP_MOD;
        s_inode_map = realloc(s_inode_map,
            s_inode_map_size * sizeof(direntry_t *));
    }

    s_inode_map[inode] = direntry_post(CALLER_INFO de);

    s_inode_map_used = inode + 1; /* TODO: assumption */
}

direntry_t *inode_map_get (ino_t inode)
{
    direntry_t *de;


    if (inode < s_inode_map_used)
    {
        de = direntry_post(CALLER_INFO s_inode_map[inode]);
    }
    else
    {
        de = NULL;
    }


    return de;
}

ino_t inode_next (void)
{
    return s_inode_next++;
}

void inode_map_clear (void)
{
    unsigned i;


    for (i = 1; i < s_inode_map_used; ++i)
    {
        direntry_delete(CALLER_INFO s_inode_map[i]);
    }

    free(s_inode_map);
}
