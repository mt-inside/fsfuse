/*
 * Copyright (C) 2008-2013 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Indexnodes list class unit tests.
 */

#include "common.h"

#include <stdlib.h>
#include <string.h>

#include "tests.h"

#include "indexnodes/indexnodes_list_internal.h"


/* TODO: move to check.h */
static void test_string_equal( const char *a, const char *b )
{
    assert(!strcmp(a,b));
}
#define test_not_null(a) (assert(a != NULL))

void indexnodes_list_can_be_created_and_destroyed( void )
{
    /* Setup */

    /* Action */
    indexnodes_list_t *ins = indexnodes_list_new( );

    /* Assert */
    test_not_null( ins );

    /* Teardown */
    indexnodes_list_delete( ins );
}

/* TODO: really need mocks here, because I need to verify that the indexnode is
 * retained and deleted right */
void indexnodes_list_can_have_a_member( void )
{
    /* Setup */
    const char *host = "fs2.example.org";
    const char *port = "1337";
    const char *version= "0.13";
    const char *id = "d34db33f";
    indexnode_t *in = indexnode_new( CALLER_INFO host, port, version, id);

    indexnodes_list_t *ins = indexnodes_list_new( );

    /* Action */
    indexnodes_list_add( ins, in );

    /* Assert */
    test_not_null( ins );

    /* Teardown */
    indexnodes_list_delete( ins );
    indexnode_delete( CALLER_INFO in );
}

void indexnodes_list_can_have_several_members( void )
{
    /* Setup */
    const char *host1 = "fs2.example.org";
    const char *port1 = "1337";
    const char *version1 = "0.13";
    const char *id1 = "d34db33f";
    indexnode_t *in1 = indexnode_new( CALLER_INFO host1, port1, version1, id1 );

    const char *host2 = "second.indexnode.com";
    const char *port2 = "1337";
    const char *version2 = "0.14";
    const char *id2 = "80081355";
    indexnode_t *in2 = indexnode_new( CALLER_INFO host2, port2, version2, id2 );

    indexnodes_list_t *ins = indexnodes_list_new( );

    /* Action */
    indexnodes_list_add( ins, in1 );
    indexnodes_list_add( ins, in2 );

    /* Assert */
    test_not_null( ins );

    /* Teardown */
    indexnodes_list_delete( ins );
    indexnode_delete( CALLER_INFO in1 );
    indexnode_delete( CALLER_INFO in2 );
}

void indexnodes_list_holds_right_number_of_items( void )
{
    /* Setup */
    const char *host_in1 = "fs2.example.org";
    const char *port_in1 = "1337";
    const char *version_in1 = "0.13";
    const char *id_in1 = "d34db33f";
    indexnode_t *in_in1 = indexnode_new( CALLER_INFO host_in1, port_in1, version_in1, id_in1 );

    const char *host_in2 = "second.indexnode.com";
    const char *port_in2 = "1337";
    const char *version_in2 = "0.14";
    const char *id_in2 = "80081355";
    indexnode_t *in_in2 = indexnode_new( CALLER_INFO host_in2, port_in2, version_in2, id_in2 );

    indexnode_t *in_out;

    indexnodes_list_t *ins = indexnodes_list_new( );
    indexnodes_list_add( ins, in_in1 );
    indexnodes_list_add( ins, in_in2 );

    indexnodes_iterator_t *iter;
    unsigned count = 0;

    /* Action & Assert */
    for( iter = indexnodes_iterator_begin( ins );
         !indexnodes_iterator_end( iter );
         iter = indexnodes_iterator_next( iter ) )
    {
        in_out = indexnodes_iterator_current( iter );

        test_not_null( in_out );
        count++;

        indexnode_delete( CALLER_INFO in_out );
    }
    indexnodes_iterator_delete( iter );

    assert( count == 2 );

    /* Teardown */
    indexnodes_list_delete( ins );
}

/* TODO: yeah so factor dis shit out */
void indexnodes_list_returns_what_is_put_in( void )
{
    /* Setup */
    const char *host_in1 = "fs2.example.org";
    const char *port_in1 = "1337";
    const char *version_in1 = "0.13";
    const char *id_in1 = "d34db33f";
    indexnode_t *in_in1 = indexnode_new( CALLER_INFO host_in1, port_in1, version_in1, id_in1 );

    const char *host_in2 = "second.indexnode.com";
    const char *port_in2 = "1337";
    const char *version_in2 = "0.14";
    const char *id_in2 = "80081355";
    indexnode_t *in_in2 = indexnode_new( CALLER_INFO host_in2, port_in2, version_in2, id_in2 );

    indexnode_t *in_out;
    const char *host_out, *port_out, *version_out, *id_out;

    indexnodes_list_t *ins = indexnodes_list_new( );
    indexnodes_list_add( ins, in_in1 );
    indexnodes_list_add( ins, in_in2 );

    indexnodes_iterator_t *iter;

    /* Action & Assert */
    iter = indexnodes_iterator_begin( ins );
    in_out = indexnodes_iterator_current( iter );
    test_not_null( in_out );

    host_out = indexnode_host( in_out );
    port_out = indexnode_port( in_out );
    version_out = indexnode_version( in_out );
    id_out = indexnode_id( in_out );

    test_string_equal( host_out, host_in2 );
    test_string_equal( port_out, port_in2 );
    test_string_equal( version_out, version_in2 );
    test_string_equal( id_out, id_in2 );

    free_const( host_out );
    free_const( port_out );
    free_const( version_out );
    free_const( id_out );

    indexnode_delete( CALLER_INFO in_out );


    iter = indexnodes_iterator_next( iter );
    in_out = indexnodes_iterator_current( iter );
    test_not_null( in_out );

    host_out = indexnode_host( in_out );
    port_out = indexnode_port( in_out );
    version_out = indexnode_version( in_out );
    id_out = indexnode_id( in_out );

    test_string_equal( host_out, host_in1 );
    test_string_equal( port_out, port_in1 );
    test_string_equal( version_out, version_in1 );
    test_string_equal( id_out, id_in1 );

    free_const( host_out );
    free_const( port_out );
    free_const( version_out );
    free_const( id_out );

    indexnode_delete( CALLER_INFO in_out );


    iter = indexnodes_iterator_next( iter );
    assert( indexnodes_iterator_end( iter ) );

    /* Teardown */
    indexnodes_iterator_delete( iter );
    indexnodes_list_delete( ins );
}

void test_indexnodes_list( void )
{
    indexnodes_list_can_be_created_and_destroyed( );
    indexnodes_list_can_have_a_member( );
    indexnodes_list_can_have_several_members( );
    indexnodes_list_holds_right_number_of_items( );
    indexnodes_list_returns_what_is_put_in( );
}
