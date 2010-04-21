/*
 * Internal definitions for the direntry module.
 *
 * Copyright (C) Matthew Turner 2009. All rights reserved.
 *
 * $Id$
 */

#ifndef _INCLUDED_DIRENTRY_INTERNAL_H
#define _INCLUDED_DIRENTRY_INTERNAL_H

struct _direntry_t
{
    ino_t                      inode;
    char                      *base_name;
    char                      *hash;
    direntry_type_t            type;
    off_t                      size; /* st_size in struct stat is off_t */
    unsigned long              link_count;
    char                      *href;

    struct _direntry_t        *children;
    int                        looked_for_children;
    unsigned                   ref_count;
    pthread_mutex_t           *lock;
    struct _direntry_t        *next;
    struct _direntry_t        *parent;
};

struct _listing_t
{
    char                      *name;
    char                      *hash;
    direntry_type_t            type;
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


extern direntry_t *direntry_new_root (CALLER_DECL_ONLY);

#endif /* _INCLUDED_DIRENTRY_INTERNAL_H */
