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
 *
 * TODO: test:
 * - passing incorrect versions
 * - _equals()
 * - _make_url()
 * - _seen & _still_valid (somehow)
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

START_TEST( indexnode_is_sane_property_bag )
{
    /* Action */
    indexnode_t *in = get_indexnode_stub( CALLER_INFO_ONLY );

    /* Assert */
    fail_unless( indexnode_equals_stub( in ), "indexnode should equal stub" );

    /* Teardown */
    indexnode_delete( CALLER_INFO in );
}
END_TEST

START_TEST( indexnode_can_be_copied_and_copy_outlives_original )
{
    /* Setup */
    indexnode_t *in1 = get_indexnode_stub( CALLER_INFO_ONLY );

    /* Action */
    indexnode_t *in2 = indexnode_copy( CALLER_INFO in1 );
    indexnode_delete( CALLER_INFO in1 );

    /* Assert */
    fail_unless( indexnode_equals_stub( in2 ), "indexnode copy should equal (deleted) original" );

    /* Teardown */
    indexnode_delete( CALLER_INFO in2 );
}
END_TEST

START_TEST( indexnode_can_be_copied_and_has_right_data )
{
    /* Setup */
    indexnode_t *in1 = get_indexnode_stub( CALLER_INFO_ONLY );

    /* Action */
    indexnode_t *in2 = indexnode_copy( CALLER_INFO in1 );

    /* Assert */
    fail_unless( indexnode_equals_stub( in1 ), "original indexnode should be sane" );
    fail_unless( indexnode_equals_stub( in2 ), "copied indexnode should be same as original" );

    /* Teardown */
    indexnode_delete( CALLER_INFO in1 );
    indexnode_delete( CALLER_INFO in2 );
}
END_TEST

Suite *indexnode_tests( void )
{
    Suite *s = suite_create( "indexnode" );

    TCase *tc_lifecycle = tcase_create( "lifecycle" );
    tcase_add_test( tc_lifecycle, indexnode_can_be_created_and_destroyed );
    tcase_add_test( tc_lifecycle, indexnode_is_sane_property_bag );
    tcase_add_test( tc_lifecycle, indexnode_can_be_copied_and_copy_outlives_original );
    tcase_add_test( tc_lifecycle, indexnode_can_be_copied_and_has_right_data );

    suite_add_tcase( s, tc_lifecycle );

    return s;
}
