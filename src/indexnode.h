/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * Indexnode class.
 */

#ifndef _INCLUDED_INDEXNODE_H
#define _INCLUDED_INDEXNODE_H

#include "nativefs.h"
#include "trace.h"

TRACE_DECLARE(indexnode)
#define indexnode_trace(...) TRACE(indexnode,__VA_ARGS__)
#define indexnode_trace_indent() TRACE_INDENT(indexnode)
#define indexnode_trace_dedent() TRACE_DEDENT(indexnode)


typedef struct _indexnode_t indexnode_t;

typedef void (*indexnode_stats_cb_t)( void *ctxt, unsigned long files, unsigned long bytes );


extern indexnode_t *indexnode_new(
    CALLER_DECL
    const char *host,
    const char *port,
    const char *version,
    const char *id
);

extern indexnode_t *indexnode_copy( CALLER_DECL indexnode_t *in);
extern void indexnode_delete( CALLER_DECL indexnode_t *in );

extern int indexnode_equals( indexnode_t *in, const char *id );

extern char *indexnode_tostring( indexnode_t *in );

extern int indexnode_tryget_listing( indexnode_t *in, const char *path, nativefs_entry_found_cb_t entry_cb, void *entry_ctxt );
extern int indexnode_tryget_alternatives( indexnode_t *in, char *hash, nativefs_entry_found_cb_t entry_cb, void *entry_ctxt );
extern int indexnode_tryget_stats( indexnode_t *in, indexnode_stats_cb_t stats_cb, void *stats_ctxt );

#endif /* _INCLUDED_INDEXNODE_H */
