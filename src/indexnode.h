/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * Indexnode class.
 */

#ifndef _INCLUDED_INDEXNODE_H
#define _INCLUDED_INDEXNODE_H

#include "proto_indexnode.h"

#include "trace.h"

TRACE_DECLARE(indexnode)
#define indexnode_trace(...) TRACE(indexnode,__VA_ARGS__)
#define indexnode_trace_indent() TRACE_INDENT(indexnode)
#define indexnode_trace_dedent() TRACE_DEDENT(indexnode)


typedef struct _indexnode_t indexnode_t;


extern indexnode_t *indexnode_new(
    CALLER_DECL
    const char * const host,
    const char * const port,
    const char * const version,
    const char * const id
);
extern indexnode_t *indexnode_from_proto(
    CALLER_DECL
    const proto_indexnode_t * const pin,
    const char * const version,
    const char * const id
);

extern indexnode_t *indexnode_post( CALLER_DECL indexnode_t * const in);
extern void indexnode_delete( CALLER_DECL indexnode_t * const in );

extern const char *indexnode_host   ( const indexnode_t * const in );
extern const char *indexnode_port   ( const indexnode_t * const in );
extern const char *indexnode_version( const indexnode_t * const in );
/* NULL if we can't get an ID, e.g. it's statically configured */
extern const char *indexnode_id     ( const indexnode_t * const in );

/* TODO: shouldn't be here (see proto_indexnode) */
extern const char *indexnode_make_url(
    const indexnode_t * const in,
    const char * const path_prefix,
    const char * const resource
);

extern void indexnode_seen( indexnode_t * const in );
extern int indexnode_still_valid( const indexnode_t * const in );

#endif /* _INCLUDED_INDEXNODE_H */
