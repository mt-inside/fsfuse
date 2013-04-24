/*
 * Copyright (C) 2008-2013 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Config system unit tests.
 */

#include "common.h"

#include <stdlib.h>
#include <string.h>

#include <check.h>
#include "tests.h"

#include "config_manager.h"
#include "config_reader.h"


START_TEST( config_defaults_are_sane )
{
    /* Setup */
    config_manager_t *mgr = config_singleton_get( );

    /* Assert */
    fail_unless( mgr != NULL, "config manager should be non-null" );

    /* Teardown */
    config_singleton_delete( mgr );
}
END_TEST

Suite *config_tests( void )
{
    Suite *s = suite_create( "config" );

    TCase *tc_lifecycle = tcase_create( "stacking" );
    tcase_add_test( tc_lifecycle, config_defaults_are_sane );

    suite_add_tcase( s, tc_lifecycle );

    return s;
}
