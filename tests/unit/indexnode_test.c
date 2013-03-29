/*
 * Copyright (C) 2008-2013 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Indexnode class unit tests.
 */

#include "common.h"

#include <stdlib.h>
#include <string.h>

#include <check.h>
#include "tests.h"
#include "indexnode_stubs.h"

#include "indexnode.h"


START_TEST( indexnode_can_be_created_and_destroyed )
{
    /* Setup */
    indexnode_t *in = get_indexnode_stub( CALLER_INFO_ONLY );

    /* Assert */
    fail_unless( in != NULL, "indexnode should be non-null" );

    /* Teardown */
    indexnode_delete( CALLER_INFO in );
}
END_TEST

START_TEST( indexnode_can_be_created_from_proto )
{
    /* Setup */
    const proto_indexnode_t *pin = get_proto_indexnode_stub( );

    /* Action */
    indexnode_t *in = indexnode_from_proto( CALLER_INFO pin, strdup( indexnode_stub_version ), strdup( indexnode_stub_id ) );

    /* Assert */
    fail_unless( in != NULL, "indexnode should be non-null" );
    fail_unless( test_equals_stub( in ), "indexnode data should match stub" );

    /* Teardown */
    indexnode_delete( CALLER_INFO in );
}
END_TEST

START_TEST( indexnode_is_sane_property_bag )
{
    /* Action */
    indexnode_t *in = get_indexnode_stub( CALLER_INFO_ONLY );

    /* Assert */
    assert( test_equals_stub( in ) );

    /* Teardown */
    indexnode_delete( CALLER_INFO in );
}
END_TEST

START_TEST( indexnode_can_be_copied_and_copy_outlives_original )
{
    /* Setup */
    indexnode_t *in1 = get_indexnode_stub( CALLER_INFO_ONLY );

    /* Action */
    indexnode_t *in2 = indexnode_post( CALLER_INFO in1 );
    indexnode_delete( CALLER_INFO in1 );

    /* Assert */
    assert( test_equals_stub( in2 ) );

    /* Teardown */
    indexnode_delete( CALLER_INFO in2 );
}
END_TEST

START_TEST( indexnode_can_be_copied_and_has_right_data )
{
    /* Setup */
    indexnode_t *in1 = get_indexnode_stub( CALLER_INFO_ONLY );

    /* Action */
    indexnode_t *in2 = indexnode_post( CALLER_INFO in1 );

    /* Assert */
    assert( test_equals_stub( in1 ) );
    assert( test_equals_stub( in2 ) );

    /* Teardown */
    indexnode_delete( CALLER_INFO in1 );
    indexnode_delete( CALLER_INFO in2 );
}
END_TEST

/* TODO: test seen and still valid. Don't make the unit tests sleep...
 * Don't test make_url as it shouldn't be here.
 * Move make_url */
Suite *indexnode_tests( void )
{
    Suite *s = suite_create( "indexnode" );

    TCase *tc_lifecycle = tcase_create( "lifecycle" );
    tcase_add_test( tc_lifecycle, indexnode_can_be_created_and_destroyed );
    tcase_add_test( tc_lifecycle, indexnode_can_be_created_from_proto );
    tcase_add_test( tc_lifecycle, indexnode_is_sane_property_bag );
    tcase_add_test( tc_lifecycle, indexnode_can_be_copied_and_copy_outlives_original );
    tcase_add_test( tc_lifecycle, indexnode_can_be_copied_and_has_right_data );

    suite_add_tcase( s, tc_lifecycle );

    return s;
}
