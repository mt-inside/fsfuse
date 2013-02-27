/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * Proto-indexnode class.
 *
 * A proto-indexnode is one about which we are aware (e.g. it has advertised
 * itself) but don't have full details of yet and thus can't use
 *
 * TODO: too many concerns - shouldn't know about the implementation of CURL,
 * should just know about IFetchShitFromTheInternet
 *   e.g. no way this should be making URIs for resorces - IFetchShit should be
 *   doing that.
 */

#ifndef _INCLUDED_PROTO_INDEXNODE_H
#define _INCLUDED_PROTO_INDEXNODE_H

typedef struct _proto_indexnode_t proto_indexnode_t;


extern const proto_indexnode_t *proto_indexnode_new( const char * const host, const char * const port );
extern void proto_indexnode_delete( const proto_indexnode_t * const pin );

extern const char *proto_indexnode_host( const proto_indexnode_t * const pin );
extern const char *proto_indexnode_port( const proto_indexnode_t * const pin );

extern const char *proto_indexnode_make_url (
    const proto_indexnode_t * const pin,
    const char * const path_prefix,
    const char * const resource
);

#endif /* _INCLUDED_PROTO_INDEXNODE_H */