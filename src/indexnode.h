/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * Indexnode class.
 */

#ifndef _INCLUDED_INDEXNODE_H
#define _INCLUDED_INDEXNODE_H

#include "trace.h"

TRACE_DECLARE(indexnode)
#define indexnode_trace(...) TRACE(indexnode,__VA_ARGS__)
#define indexnode_trace_indent() TRACE_INDENT(indexnode)
#define indexnode_trace_dedent() TRACE_DEDENT(indexnode)


typedef struct _indexnode_t indexnode_t;


extern indexnode_t *indexnode_new(
    CALLER_DECL
    const char *host,
    const char *port,
    const char *version,
    const char *id
);

extern indexnode_t *indexnode_post( CALLER_DECL indexnode_t *in);
extern void indexnode_delete( CALLER_DECL indexnode_t *in );

extern int indexnode_equals( indexnode_t *in, const char *id );

extern char *indexnode_tostring( indexnode_t *in );

#include "listing.h"

/* TODO: shouldn't be here (see proto_indexnode) */
extern const char *indexnode_make_url(
    const indexnode_t *in,
    const char *path_prefix,
    const char *resource
);

extern int indexnode_tryget_listing( indexnode_t *in, const char *path, listing_list_t **lis );
extern int indexnode_tryget_best_alternative( indexnode_t *in, char *hash, listing_t **li_best );

#endif /* _INCLUDED_INDEXNODE_H */
