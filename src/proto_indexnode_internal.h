/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * Proto-indexnode struct definition.
 */

#ifndef _INCLUDED_PROTO_INDEXNODE_INTERNAL_H
#define _INCLUDED_PROTO_INDEXNODE_INTERNAL_H

/* essentially Pair<host, port> */
struct _proto_indexnode_t
{
    const char * host;
    const char * port;
};

#endif /* _INCLUDED_PROTO_INDEXNODE_INTERNAL_H */
