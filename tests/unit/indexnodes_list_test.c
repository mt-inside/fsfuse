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

#include <check.h>
#include "tests.h"
#include "indexnode_stubs.h"

#include "indexnodes/indexnodes_list_internal.h"


START_TEST( indexnodes_list_can_be_created_and_destroyed )
{
    /* Setup */

    /* Action */
    indexnodes_list_t *ins = indexnodes_list_new( );

    /* Assert */
    fail_unless( ins != NULL, "list should be non-null" );

    /* Teardown */
    indexnodes_list_delete( ins );
}
END_TEST

/* TODO: really need mocks here, because I need to verify that the indexnode is
 * retained and deleted right */
START_TEST( indexnodes_list_can_have_a_member )
{
    /* Setup */
    indexnode_t *in = get_indexnode_stub( CALLER_INFO_ONLY );

    indexnodes_list_t *ins = indexnodes_list_new( );

    /* Action */
    indexnodes_list_add( ins, indexnode_post( CALLER_INFO in ) );

    /* Assert */
    fail_unless( ins != NULL, "list should be non-null" );

    /* Teardown */
    indexnodes_list_delete( ins );
    indexnode_delete( CALLER_INFO in );
}
END_TEST

START_TEST( indexnodes_list_can_have_several_members )
{
    /* Setup */
    indexnode_t *in1 = get_indexnode_stub( CALLER_INFO_ONLY );
    indexnode_t *in2 = get_indexnode_stub2( CALLER_INFO_ONLY );
    indexnodes_list_t *ins = indexnodes_list_new( );

    /* Action */
    indexnodes_list_add( ins, indexnode_post( CALLER_INFO in1 ) );
    indexnodes_list_add( ins, indexnode_post( CALLER_INFO in2 ) );

    /* Assert */
    fail_unless( ins != NULL, "list should be non-null" );

    /* Teardown */
    indexnodes_list_delete( ins );
    indexnode_delete( CALLER_INFO in1 );
    indexnode_delete( CALLER_INFO in2 );
}
END_TEST

START_TEST( indexnodes_list_holds_right_number_of_items )
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

        fail_unless( in_out != NULL, "iterator should return non-null indexnode" );
        count++;

        indexnode_delete( CALLER_INFO in_out );
    }
    indexnodes_iterator_delete( iter );

    fail_unless( count == 2, "should get 2 indexnodes from list" );

    /* Teardown */
    indexnodes_list_delete( ins );
    indexnode_delete( CALLER_INFO in_in1 );
    indexnode_delete( CALLER_INFO in_in2 );
}
END_TEST

START_TEST( indexnodes_list_returns_what_is_put_in )
{
    /* Setup */
    indexnode_t *in_in1 = get_indexnode_stub( CALLER_INFO_ONLY );
    indexnode_t *in_in2 = get_indexnode_stub2( CALLER_INFO_ONLY );
    indexnode_t *in_out;

    indexnodes_list_t *ins = indexnodes_list_new( );
    indexnodes_list_add( ins, indexnode_post( CALLER_INFO in_in1 ) );
    indexnodes_list_add( ins, indexnode_post( CALLER_INFO in_in2 ) );

    indexnodes_iterator_t *iter;

    /* Action & Assert */
    iter = indexnodes_iterator_begin( ins );
    in_out = indexnodes_iterator_current( iter );
    fail_unless( in_out != NULL, "iterator should give non-null indexnode" );

    fail_unless( indexnode_equals_stub(  in_out ) ||
                 indexnode_equals_stub2( in_out ),
                 "indexnode should be one stub or the other" );

    indexnode_delete( CALLER_INFO in_out );


    iter = indexnodes_iterator_next( iter );
    in_out = indexnodes_iterator_current( iter );
    fail_unless( in_out != NULL, "iterator should give non-null indexnode" );

    fail_unless( indexnode_equals_stub(  in_out ) ||
                 indexnode_equals_stub2( in_out ),
                 "indexnode should be one stub or the other" );

    indexnode_delete( CALLER_INFO in_out );


    iter = indexnodes_iterator_next( iter );
    fail_unless( indexnodes_iterator_end( iter ) != 0, "iterator should be at end" );

    /* Teardown */
    indexnodes_iterator_delete( iter );
    indexnodes_list_delete( ins );
    indexnode_delete( CALLER_INFO in_in1 );
    indexnode_delete( CALLER_INFO in_in2 );
}
END_TEST

Suite *indexnodes_list_tests( void )
{
    Suite *s = suite_create( "indexnodes_list" );


    TCase *tc_lifecycle = tcase_create( "lifecycle" );
    tcase_add_test( tc_lifecycle, indexnodes_list_can_be_created_and_destroyed );
    suite_add_tcase( s, tc_lifecycle );

    TCase *tc_items = tcase_create( "items" );
    tcase_add_test( tc_items, indexnodes_list_can_have_a_member );
    tcase_add_test( tc_items, indexnodes_list_can_have_several_members );
    tcase_add_test( tc_items, indexnodes_list_holds_right_number_of_items );
    tcase_add_test( tc_items, indexnodes_list_returns_what_is_put_in );
    suite_add_tcase( s, tc_items );


    return s;
}
