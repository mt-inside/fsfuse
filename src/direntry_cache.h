/*
 * Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
 *
 * Caching mechanism for direntrys and direntry trees - API definition.
 */

#ifndef _INCLUDED_DIRENTRY_CACHE_H
#define _INCLUDED_DIRENTRY_CACHE_H

#include "direntry.h"


TRACE_DECLARE(direntry_cache)
#define direntry_cache_trace(...) TRACE(direntry_cache,__VA_ARGS__)
#define direntry_cache_trace_indent() TRACE_INDENT(direntry_cache)
#define direntry_cache_trace_dedent() TRACE_DEDENT(direntry_cache)


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
