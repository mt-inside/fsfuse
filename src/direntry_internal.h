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
    /* External properties */

    char                      *base_name;
    /* These are stored normalised. Fuse doesn't give us ".."s, but we don't
     * generate e.g. "//". It would be easier in places to not care if we make
     * non-normal paths, but the overhead of converting them might as well be
     * (once) before storage rather than (many times) afterwards. */
    char                      *path;
    char                      *hash;
    direntry_type_t            type;
    off_t                      size; /* st_size in struct stat is off_t */
    unsigned long              link_count;
#if FEATURE_DIRENTRY_CACHE
    time_t                     cache_last_valid;
#endif
    char                      *href;

    /* Private properties */
    struct _direntry_t        *children; /* directories in fs2 are never empty,
                                            so NULL children means we haven't
                                            looked yet */
    int                        looked_for_children;
    unsigned                   ref_count;
    pthread_mutex_t           *lock;
    struct _direntry_t        *next;
    struct _direntry_t        *parent;
};


extern direntry_t *direntry_new_root (void);

#endif /* _INCLUDED_DIRENTRY_INTERNAL_H */
