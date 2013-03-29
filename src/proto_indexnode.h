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


extern proto_indexnode_t *proto_indexnode_new( const char * const host, const char * const port );
extern void proto_indexnode_delete( proto_indexnode_t *pin );

extern const char *proto_indexnode_host( proto_indexnode_t *pin );
extern const char *proto_indexnode_port( proto_indexnode_t *pin );

extern const char *proto_indexnode_make_url (
    proto_indexnode_t *pin,
    const char *path_prefix,
    const char *resource
);

#endif /* _INCLUDED_PROTO_INDEXNODE_H */
