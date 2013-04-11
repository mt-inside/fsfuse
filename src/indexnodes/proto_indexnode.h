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
typedef struct _proto_indexnode_t
{
    const char * host;
    const char * port;
} proto_indexnode_t;


extern proto_indexnode_t *proto_indexnode_new( const char *host, const char *port );
extern void proto_indexnode_init(
    proto_indexnode_t *pin,
    const char *host,
    const char *port
);

extern void proto_indexnode_delete( proto_indexnode_t *pin );
extern void proto_indexnode_teardown( proto_indexnode_t *pin );

extern const char *proto_indexnode_host( const proto_indexnode_t *pin );
extern const char *proto_indexnode_port( const proto_indexnode_t *pin );

extern int proto_indexnode_get_info( proto_indexnode_t *pin,
                                     const char **protocol,
                                     const char **id );

extern const char *proto_indexnode_make_url (
    proto_indexnode_t *pin,
    const char *path_prefix,
    const char *resource
);

#endif /* _INCLUDED_PROTO_INDEXNODE_INTERNAL_H */
