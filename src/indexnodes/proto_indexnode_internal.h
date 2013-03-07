/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * Proto-indexnode class internal header.
 */

#ifndef _INCLUDED_PROTO_INDEXNODE_INTERNAL_H
#define _INCLUDED_PROTO_INDEXNODE_INTERNAL_H

#include "common.h"

#include "proto_indexnode.h"


/* essentially Pair<host, port> */
struct _proto_indexnode_t
{
    const char * host;
    const char * port;
};

extern void proto_indexnode_init(
    proto_indexnode_t * const pin,
    const char * const host,
    const char * const port
);
extern void proto_indexnode_teardown( const proto_indexnode_t * const pin );

#endif /* _INCLUDED_PROTO_INDEXNODE_INTERNAL_H */
