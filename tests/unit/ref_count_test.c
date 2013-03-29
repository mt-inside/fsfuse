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
#include "ref_count.h"


START_TEST( ref_count_can_be_created_and_destroyed )
{
    /* Setup */

    /* Action */
    ref_count_t *refc = ref_count_new( );

    /* Assert */
    fail_unless( refc != NULL, "new ref counter should be non-null" );

    /* Teardown */
    ref_count_dec( refc );
    ref_count_delete( refc );
}
END_TEST

START_TEST( ref_count_can_be_inc_and_dec )
{
    /* Setup */

    /* Action */
    ref_count_t *refc = ref_count_new( );

    /* Assert */
    fail_unless( ref_count_inc( refc ) == 2, "ref count should be correct" );
    fail_unless( ref_count_inc( refc ) == 3, "ref count should be correct");
    fail_unless( ref_count_inc( refc ) == 4, "ref count should be correct" );
    fail_unless( ref_count_dec( refc ) == 3, "ref count should be correct" );
    fail_unless( ref_count_dec( refc ) == 2, "ref count should be correct" );
    fail_unless( ref_count_dec( refc ) == 1, "ref count should be correct" );
    fail_unless( ref_count_dec( refc ) == 0, "ref count should be correct" );

    /* Teardown */
    ref_count_delete( refc );
}
END_TEST

Suite *ref_count_tests( void )
{
    Suite *s = suite_create( "ref_count" );

    TCase *tc_core = tcase_create( "core" );
    tcase_add_test( tc_core, ref_count_can_be_created_and_destroyed );
    tcase_add_test( tc_core, ref_count_can_be_inc_and_dec );

    suite_add_tcase( s, tc_core );

    return s;
}
