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

#include <check.h>
#include "tests.h"
#include "indexnode_stubs.h"

#include "proto_indexnode.h"


START_TEST( proto_indexnode_can_be_created_and_destroyed )
{
    /* Action */
    const proto_indexnode_t *pin = get_proto_indexnode_stub( );

    /* Assert */
    fail_unless( pin != NULL, "new proto_indexnode should be non-null" );

    /* Teardown */
    proto_indexnode_delete( pin );
}
END_TEST

START_TEST( proto_indexnode_is_sane_property_bag )
{
    /* Action */
    const proto_indexnode_t *pin = get_proto_indexnode_stub( );

    /* Assert */
    fail_unless( proto_indexnode_equals_stub( pin ), "proto_indexnode should equal stub" );

    /* Teardown */
    proto_indexnode_delete( pin );
}
END_TEST

Suite *proto_indexnode_tests( void )
{
    Suite *s = suite_create( "proto_indexnode" );

    TCase *tc_core = tcase_create( "core" );
    tcase_add_test( tc_core, proto_indexnode_is_sane_property_bag );
    tcase_add_test( tc_core, proto_indexnode_can_be_created_and_destroyed );

    suite_add_tcase( s, tc_core );

    return s;
}
