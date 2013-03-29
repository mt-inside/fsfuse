/*
 * Copyright (C) 2008-2013 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Proto-indexnode class unit tests
 */

#include "common.h"

#include <stdlib.h>
#include <string.h>

#include "tests.h"
#include "indexnode_stubs.h"

#include "proto_indexnode.h"


/* TODO: move to check.h */
static void test_string_equal( const char *a, const char *b )
{
    assert(!strcmp(a,b));
}
#define test_not_null(a) (assert(a != NULL))

void proto_indexnode_can_be_created_and_destroyed( void )
{
    /* Action */
    const proto_indexnode_t *pin = get_proto_indexnode_stub( );

    /* Assert */
    test_not_null( pin );

    /* Teardown */
    proto_indexnode_delete( pin );
}

void proto_indexnode_is_sane_property_bag( void )
{
    /* Setup */
    const char *host_out, *port_out;

    /* Action */
    const proto_indexnode_t *pin = get_proto_indexnode_stub( );

    /* Assert */
    host_out = proto_indexnode_host( pin );
    port_out = proto_indexnode_port( pin );

    test_string_equal( host_out, indexnode_stub_host );
    test_string_equal( port_out, indexnode_stub_port );

    free_const( host_out );
    free_const( port_out );

    /* Teardown */
    proto_indexnode_delete( pin );
}

void test_proto_indexnode( void )
{
    proto_indexnode_is_sane_property_bag( );
    proto_indexnode_can_be_created_and_destroyed( );
}
