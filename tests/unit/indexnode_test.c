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
#include "indexnode.h"


/* TODO: move to check.h */
static void test_string_equal( const char *a, const char *b )
{
    assert(!strcmp(a,b));
}
#define test_not_null(a) (assert(a != NULL))

void indexnode_can_be_created_and_destroyed( void )
{
    /* Setup */
    const char *host_in = "fs2.example.org";
    const char *port_in = "1337";
    const char *version_in = "0.13";
    const char *id_in = "d34db33f";

    /* Action */
    indexnode_t *in = indexnode_new( host_in, port_in, version_in, id_in );

    /* Assert */
    test_not_null( in );

    /* Teardown */
    indexnode_delete( in );
}

void indexnode_can_be_created_from_proto( void )
{
    /* Setup */
    const char *host_in = "fs2.example.org";
    const char *port_in = "1337";
    const char *version_in = "0.13";
    const proto_indexnode_t *pin = proto_indexnode_new( host_in, port_in );

    /* Action */
    indexnode_t *in = indexnode_from_proto( pin, version_in );

    /* Assert */
    test_not_null( in );

    /* Teardown */
    indexnode_delete( in );
}

void indexnode_is_sane_property_bag( void )
{
    /* Setup */
    const char *host_in = "fs2.example.org";
    const char *port_in = "1337";
    const char *version_in = "0.13";
    const char *id_in = "d34db33f";
    const char *host_out, *port_out, *version_out, *id_out;

    /* Action */
    indexnode_t *in = indexnode_new( host_in, port_in, version_in, id_in );

    /* Assert */
    host_out = indexnode_host( in );
    port_out = indexnode_port( in );
    version_out = indexnode_version( in );
    id_out = indexnode_id( in );

    test_string_equal( host_out, host_in );
    test_string_equal( port_out, port_in );
    test_string_equal( version_out, version_in );
    test_string_equal( id_out, id_in );

    free_const( host_out );
    free_const( port_out );
    free_const( version_out );
    free_const( id_out );

    /* Teardown */
    indexnode_delete( in );
}

void indexnode_can_be_copied_and_copy_outlives_original( void )
{
    /* Setup */
    const char *host_in = "fs2.example.org";
    const char *port_in = "1337";
    const char *version_in = "0.13";
    const char *id_in = "d34db33f";
    const char *host_out2;
    indexnode_t *in1 = indexnode_new( host_in, port_in, version_in, id_in );

    /* Action */
    indexnode_t *in2 = indexnode_post( in1 );
    indexnode_delete( in1 );

    /* Assert */
    host_out2 = indexnode_host( in2 );

    test_string_equal( host_out2, host_in );

    free_const( host_out2 );

    /* Teardown */
    indexnode_delete( in2 );
}

void indexnode_can_be_copied_and_has_right_data( void )
{
    /* Setup */
    const char *host_in = "fs2.example.org";
    const char *port_in = "1337";
    const char *version_in = "0.13";
    const char *id_in = "d34db33f";
    const char *host_out1, *host_out2;
    indexnode_t *in1 = indexnode_new( host_in, port_in, version_in, id_in );

    /* Action */
    indexnode_t *in2 = indexnode_post( in1 );

    /* Assert */
    host_out1 = indexnode_host( in1 );
    host_out2 = indexnode_host( in2 );

    test_string_equal( host_out2, host_out1 );
    test_string_equal( host_out2, host_in );

    free_const( host_out1 );
    free_const( host_out2 );

    /* Teardown */
    indexnode_delete( in1 );
    indexnode_delete( in2 );
}

/* TODO: test seen and still valid. Don't make the unit tests sleep...
 * Don't test make_url as it shouldn't be here.
 * Move make_url */

void test_indexnode( void )
{
    indexnode_can_be_created_and_destroyed( );
    indexnode_can_be_created_from_proto( );
    indexnode_is_sane_property_bag( );
    indexnode_can_be_copied_and_copy_outlives_original( );
    indexnode_can_be_copied_and_has_right_data( );
}
