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

#include "tests.h"
#include "indexnode_stubs.h"

#include "indexnode.h"


#define test_not_null(a) (assert(a != NULL))

void indexnode_can_be_created_and_destroyed( void )
{
    /* Setup */
    indexnode_t *in = get_indexnode_stub( CALLER_INFO_ONLY );

    /* Assert */
    test_not_null( in );

    /* Teardown */
    indexnode_delete( CALLER_INFO in );
}

void indexnode_can_be_created_from_proto( void )
{
    /* Setup */
    const proto_indexnode_t *pin = get_proto_indexnode_stub( );

    /* Action */
    indexnode_t *in = indexnode_from_proto( CALLER_INFO pin, strdup( indexnode_stub_version ), strdup( indexnode_stub_id ) );

    /* Assert */
    test_not_null( in );
    assert( test_equals_stub( in ) );

    /* Teardown */
    indexnode_delete( CALLER_INFO in );
}

void indexnode_is_sane_property_bag( void )
{
    /* Action */
    indexnode_t *in = get_indexnode_stub( CALLER_INFO_ONLY );

    /* Assert */
    assert( test_equals_stub( in ) );

    /* Teardown */
    indexnode_delete( CALLER_INFO in );
}

void indexnode_can_be_copied_and_copy_outlives_original( void )
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

void indexnode_can_be_copied_and_has_right_data( void )
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

/* TODO: test seen and still valid. Don't make the unit tests sleep...
 * Don't test make_url as it shouldn't be here.
 * Move make_url */

void test_indexnode( void )
{
    indexnode_trace_on( );

    indexnode_can_be_created_and_destroyed( );
    indexnode_can_be_created_from_proto( );
    indexnode_is_sane_property_bag( );
    indexnode_can_be_copied_and_copy_outlives_original( );
    indexnode_can_be_copied_and_has_right_data( );

    indexnode_trace_off( );
}
