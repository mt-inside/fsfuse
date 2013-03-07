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


/* TODO: move to check.h */
#define test_not_null(a) (assert(a != NULL))

void ref_count_can_be_created_and_destroyed( void )
{
    /* Setup */

    /* Action */
    ref_count_t *refc = ref_count_new( );

    /* Assert */
    test_not_null( refc );

    /* Teardown */
    ref_count_dec( refc );
    ref_count_delete( refc );
}

void ref_count_can_be_inc_and_dec( void )
{
    /* Setup */

    /* Action */
    ref_count_t *refc = ref_count_new( );

    /* Assert */
    test_not_null( refc );
    assert( ref_count_inc( refc ) == 2 );
    assert( ref_count_inc( refc ) == 3 );
    assert( ref_count_inc( refc ) == 4 );
    assert( ref_count_dec( refc ) == 3 );
    assert( ref_count_dec( refc ) == 2 );
    assert( ref_count_dec( refc ) == 1 );
    assert( ref_count_dec( refc ) == 0 );

    /* Teardown */
    ref_count_delete( refc );
}

void test_ref_count( void )
{
    ref_count_can_be_created_and_destroyed( );
    ref_count_can_be_inc_and_dec( );
}
