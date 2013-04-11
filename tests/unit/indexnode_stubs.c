/*
 * Copyright (C) 2008-2013 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Indexnode stubs for testing.
 */

#include "common.h"

#include <stdlib.h>
#include <string.h>

#include "indexnode_stubs.h"


const char *indexnode_stub_host = "fs2.example.org";
const char *indexnode_stub_port = "1337";
const char *indexnode_stub_version = "0.13";
const char *indexnode_stub_id = "d34db33f";

proto_indexnode_t *get_proto_indexnode_stub( void )
{
    return proto_indexnode_new(
        strdup( indexnode_stub_host ),
        strdup( indexnode_stub_port )
    );
}

indexnode_t *get_indexnode_stub( CALLER_DECL_ONLY )
{
    return indexnode_new(
        CALLER_PASS
        strdup( indexnode_stub_host ),
        strdup( indexnode_stub_port ),
        strdup( indexnode_stub_version ),
        strdup( indexnode_stub_id )
    );
}


const char *indexnode_stub_host2 = "second.indexnode.com";
const char *indexnode_stub_port2 = "1337";
const char *indexnode_stub_version2 = "0.14";
const char *indexnode_stub_id2 = "80081355";

indexnode_t *get_indexnode_stub2( CALLER_DECL_ONLY )
{
    return indexnode_new(
        CALLER_PASS
        strdup( indexnode_stub_host2 ),
        strdup( indexnode_stub_port2 ),
        strdup( indexnode_stub_version2 ),
        strdup( indexnode_stub_id2 )
    );
}


int proto_indexnode_equals_stub( proto_indexnode_t *pin )
{
    const char *host_out, *port_out;
    int rc;

    host_out = proto_indexnode_host( pin );
    port_out = proto_indexnode_port( pin );

    rc = !strcmp( host_out, indexnode_stub_host ) &&
         !strcmp( port_out, indexnode_stub_port );

    free_const( host_out );
    free_const( port_out );

    return rc;
}

int indexnode_equals_stub( indexnode_t *in )
{
    const char *test_uri;
    int rc;

    test_uri = indexnode_make_url( in, strdup( "foo" ), strdup( "bar" ) );
    rc = !strcmp( test_uri, "http://fs2.example.org:1337/foo/bar" );
    free_const( test_uri );

    return rc;
}

int indexnode_equals_stub2( indexnode_t *in )
{
    const char *test_uri;
    int rc;

    test_uri = indexnode_make_url( in, strdup( "foo" ), strdup( "bar" ) );
    rc = !strcmp( test_uri, "http://second.indexnode.com:1337/foo/bar" );
    free_const( test_uri );

    return rc;
}
