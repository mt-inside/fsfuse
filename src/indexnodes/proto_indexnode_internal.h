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
    proto_indexnode_t *pin,
    const char *host,
    const char *port
);
extern void proto_indexnode_teardown( proto_indexnode_t *pin );

#endif /* _INCLUDED_PROTO_INDEXNODE_INTERNAL_H */
