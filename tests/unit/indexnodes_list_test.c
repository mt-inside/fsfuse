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
#include "indexnode_stubs.h"

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
    indexnode_t *in = get_indexnode_stub( CALLER_INFO_ONLY );

    indexnodes_list_t *ins = indexnodes_list_new( );

    /* Action */
    indexnodes_list_add( ins, indexnode_post( CALLER_INFO in ) );

    /* Assert */
    test_not_null( ins );

    /* Teardown */
    indexnodes_list_delete( ins );
    indexnode_delete( CALLER_INFO in );
}

void indexnodes_list_can_have_several_members( void )
{
    /* Setup */
    indexnode_t *in1 = get_indexnode_stub( CALLER_INFO_ONLY );
    indexnode_t *in2 = get_indexnode_stub2( CALLER_INFO_ONLY );
    indexnodes_list_t *ins = indexnodes_list_new( );

    /* Action */
    indexnodes_list_add( ins, indexnode_post( CALLER_INFO in1 ) );
    indexnodes_list_add( ins, indexnode_post( CALLER_INFO in2 ) );

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
    indexnode_t *in_in1 = get_indexnode_stub( CALLER_INFO_ONLY );
    indexnode_t *in_in2 = get_indexnode_stub2( CALLER_INFO_ONLY );
    indexnode_t *in_out;

    indexnodes_list_t *ins = indexnodes_list_new( );
    indexnodes_list_add( ins, indexnode_post( CALLER_INFO in_in1 ) );
    indexnodes_list_add( ins, indexnode_post( CALLER_INFO in_in2 ) );

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
    indexnode_delete( CALLER_INFO in_in1 );
    indexnode_delete( CALLER_INFO in_in2 );
}

/* TODO: decouple me from iterator order */
void indexnodes_list_returns_what_is_put_in( void )
{
    /* Setup */
    indexnode_t *in_in1 = get_indexnode_stub( CALLER_INFO_ONLY );
    indexnode_t *in_in2 = get_indexnode_stub2( CALLER_INFO_ONLY );
    indexnode_t *in_out;
    const char *host_out, *port_out, *version_out, *id_out;

    indexnodes_list_t *ins = indexnodes_list_new( );
    indexnodes_list_add( ins, indexnode_post( CALLER_INFO in_in1 ) );
    indexnodes_list_add( ins, indexnode_post( CALLER_INFO in_in2 ) );

    indexnodes_iterator_t *iter;

    /* Action & Assert */
    iter = indexnodes_iterator_begin( ins );
    in_out = indexnodes_iterator_current( iter );
    test_not_null( in_out );

    host_out = indexnode_host( in_out );
    port_out = indexnode_port( in_out );
    version_out = indexnode_version( in_out );
    id_out = indexnode_id( in_out );

    test_string_equal( host_out, indexnode_stub_host2 );
    test_string_equal( port_out, indexnode_stub_port2 );
    test_string_equal( version_out, indexnode_stub_version2 );
    test_string_equal( id_out, indexnode_stub_id2 );

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

    test_string_equal( host_out, indexnode_stub_host );
    test_string_equal( port_out, indexnode_stub_port );
    test_string_equal( version_out, indexnode_stub_version );
    test_string_equal( id_out, indexnode_stub_id );

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
    indexnode_delete( CALLER_INFO in_in1 );
    indexnode_delete( CALLER_INFO in_in2 );
}

void test_indexnodes_list( void )
{
    indexnode_trace_on( );

    indexnodes_list_can_be_created_and_destroyed( );
    indexnodes_list_can_have_a_member( );
    indexnodes_list_can_have_several_members( );
    indexnodes_list_holds_right_number_of_items( );
    indexnodes_list_returns_what_is_put_in( );

    indexnode_trace_off( );
}
