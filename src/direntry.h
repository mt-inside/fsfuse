/*
 * API for dealing with "direntrys" - "entries in a directory". This is
 * basically our internal file system tree implementation.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#ifndef _INCLUDED_DIRENTRY_H
#define _INCLUDED_DIRENTRY_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "trace.h"
#include "uthash.h"


TRACE_DECLARE(direntry)

typedef enum
{
    direntry_type_FILE,
    direntry_type_DIRECTORY
} direntry_type_t;

typedef struct _direntry_t
{
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

    struct _direntry_t        *children; /* directories in fs2 are never empty,
                                            so NULL children means we haven't
                                            looked yet */
    int                        looked_for_children;
    unsigned                   ref_count;
    struct _direntry_t        *next;
    UT_hash_handle             hh;
} direntry_t;


extern direntry_t *de_root;


extern int direntry_init (void);
extern void direntry_finalise (void);

extern direntry_t *direntry_new (void);
extern void direntry_post (direntry_t *de);
extern direntry_t *direntry_copy (direntry_t *de);
extern void direntry_delete (direntry_t *de);
extern void direntry_delete_with_children (direntry_t *de);

extern int direntry_get               (const char * const path, direntry_t **de);
extern int direntry_get_with_children (const char * const path, direntry_t **de);
extern int populate_directory (direntry_t *de);

extern int direntry_de2stat (struct stat *st, direntry_t *de);
extern char *fsfuse_dirname (const char *path);
extern char *fsfuse_basename (const char *path);

#endif /* _INCLUDED_DIRENTRY_H */
