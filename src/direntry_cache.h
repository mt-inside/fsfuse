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

extern int direntry_cache_init (void);
extern void direntry_cache_finalise (void);

extern int direntry_cache_add (CALLER_DECL direntry_t *de);
extern direntry_t *direntry_cache_get (const char * const path);
extern int direntry_cache_del (CALLER_DECL direntry_t *de);
extern void direntry_cache_notify_still_valid (direntry_t *de);
extern void direntry_cache_notify_stale (direntry_t *de);

extern void direntry_cache_add_children (
    direntry_t *parent,
    direntry_t *new_children
);

#endif /* _INCLUDED_DIRENTRY_CACHE_H */
