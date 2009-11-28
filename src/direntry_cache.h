/*
 * Caching mechanism for direntrys and direntry trees - API definition.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#ifndef _INCLUDED_DIRENTRY_CACHE_H
#define _INCLUDED_DIRENTRY_CACHE_H

#include "direntry.h"


TRACE_DECLARE(direntry_cache)


typedef enum
{
    direntry_cache_status_HIT,
    direntry_cache_status_NOENT,
    direntry_cache_status_UNKNOWN
} direntry_cache_status_t;


extern int direntry_cache_init (void);
extern void direntry_cache_finalise (void);

extern int direntry_cache_add (CALLER_DECL direntry_t *de);
extern void direntry_cache_add_list (direntry_t *dirents, const char *parent);
extern direntry_cache_status_t direntry_cache_get (CALLER_DECL const char * const path, direntry_t **de_out);
extern int direntry_cache_del (CALLER_DECL direntry_t *de);
extern void direntry_cache_notify_still_valid (direntry_t *de);
extern void direntry_cache_notify_stale (direntry_t *de);

extern void direntry_cache_add_children (
    direntry_t *parent,
    direntry_t *new_children
);

extern char *direntry_cache_status (const char * const path);

#endif /* _INCLUDED_DIRENTRY_CACHE_H */
