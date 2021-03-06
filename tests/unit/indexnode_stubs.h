/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * Indexnode stubs for testing.
 */

#ifndef _INCLUDED_INDEXNODE_STUBS_H
#define _INCLUDED_INDEXNODE_STUBS_H

#include "common.h"

#include "indexnode.h"
#include "indexnodes/proto_indexnode.h"

extern const char *indexnode_stub_host;
extern const char *indexnode_stub_port;
extern const char *indexnode_stub_version;
extern const char *indexnode_stub_id;
extern proto_indexnode_t *get_proto_indexnode_stub( void );
extern indexnode_t *get_indexnode_stub( CALLER_DECL_ONLY );

extern const char *indexnode_stub_host2;
extern const char *indexnode_stub_port2;
extern const char *indexnode_stub_version2;
extern const char *indexnode_stub_id2;
extern indexnode_t *get_indexnode_stub2( CALLER_DECL_ONLY );

extern int proto_indexnode_equals_stub( proto_indexnode_t *pin );
extern int indexnode_equals_stub(  indexnode_t *in );
extern int indexnode_equals_stub2( indexnode_t *in );

#endif /* _INCLUDED_INDEXNODE_STUBS_H */
