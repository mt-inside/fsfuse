/*
 * Copyright (C) 2008-2013 Matthew Turner.
 *
 * This procram is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Config system unit tests.
 * Actually some of these read files from testdata and so aren't techically unit
 * tests. In order to really unit test this is to mock out either:
 * - loading the file from disk (give the parser a string from a resource/stub
 *   file instead)
 * - libxml2 parsing, i.e. hide parsing behing an interface and separate parsing
 *   and building the config data structure. Can then feed
 *   "config_read_from_file" with canned AST.
 * However, that seems like waaay too much effort.
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
    char *alias;
    char **hosts;


    /* Setup */
    config_manager_t *mgr = config_singleton_get( );
    fail_unless( mgr != NULL, "config manager should be non-null" );

    config_reader_t *config = config_get_reader( );
    fail_unless( config != NULL, "config reader should be non-null" );

    /* Assert - check some of the values */
    alias = config_alias( config );
    ck_assert_str_eq( alias, "[fsfuse]Anonymous" );
    free( alias );

    ck_assert_int_eq( config_indexnode_autodetect_listen( config ), 1 );

    ck_assert_int_eq( config_indexnode_advert_port( config ), 42444 );

    /* empty array */
    hosts = config_indexnode_hosts( config );
    fail_unless( hosts != NULL );
    fail_unless( hosts[0] == NULL );

    /* Teardown */
    config_reader_delete( config );
    config_singleton_delete( );
}
END_TEST

START_TEST( config_can_override_from_cmdline )
{
    config_reader_t *config;


    /* Setup */
    config_manager_t *mgr = config_singleton_get( );
    fail_unless( mgr != NULL, "config manager should be non-null" );

    config = config_get_reader( );
    fail_unless( config != NULL, "config reader should be non-null" );

    ck_assert_int_eq( config_proc_fg( config ), 0 );
    ck_assert_int_eq( config_proc_singlethread( config ), 0 );
    ck_assert_int_eq( config_proc_debug( config ), 0 );

    config_reader_delete( config );

    /* Assert - check overridden values */

    config_manager_add_from_cmdline(
        1, 0, /* Set to same */
        1, 1, /* Set to different */
        0, 1  /* "unset" => argument ignored */
    );

    config = config_get_reader( );
    fail_unless( config != NULL, "config reader should be non-null" );

    ck_assert_int_eq( config_proc_fg( config ), 0 );
    ck_assert_int_eq( config_proc_singlethread( config ), 1 );
    ck_assert_int_eq( config_proc_debug( config ), 0 );

    config_reader_delete( config );

    /* Teardown */
    config_singleton_delete( );
}
END_TEST

START_TEST( config_can_override_from_file )
{
    config_reader_t *config;
    char *alias;
    char **hosts;


    /* Setup */
    config_manager_t *mgr = config_singleton_get( );
    fail_unless( mgr != NULL, "config manager should be non-null" );

    config = config_get_reader( );
    fail_unless( config != NULL, "config reader should be non-null" );

    alias = config_alias( config );
    ck_assert_str_eq( alias, "[fsfuse]Anonymous" );
    free( alias );

    ck_assert_int_eq( config_indexnode_advert_port( config ), 42444 );

    /* empty array */
    hosts = config_indexnode_hosts( config );
    fail_unless( hosts != NULL );
    fail_unless( hosts[0] == NULL );

    config_reader_delete( config );


    config_manager_add_from_file( test_isolate_file( strdup( "test_fsfuserc" ) ) );

    config = config_get_reader( );
    fail_unless( config != NULL, "config reader should be non-null" );

    /* Assert - check non-overridden values are not changed */
    ck_assert_int_eq( config_attr_mode_file( config ), 0444 );

    /* Assert - check overridden values */
    alias = config_alias( config );
    ck_assert_str_eq( alias, "test alias" );
    free( alias );

    ck_assert_int_eq( config_indexnode_advert_port( config ), 55555 );

    hosts = config_indexnode_hosts( config );
    fail_unless( hosts != NULL );
    ck_assert_str_eq( hosts[0], "test host 1" );
    ck_assert_str_eq( hosts[1], "test host 2" );
    fail_unless(      hosts[2] == NULL );

    config_reader_delete( config );

    /* Teardown */
    config_singleton_delete( );
}
END_TEST

START_TEST( config_can_override_from_several )
{
    config_reader_t *config;
    char *alias;
    char **hosts;


    /* Setup */
    config_manager_t *mgr = config_singleton_get( );
    fail_unless( mgr != NULL, "config manager should be non-null" );

    config = config_get_reader( );
    fail_unless( config != NULL, "config reader should be non-null" );

    alias = config_alias( config );
    ck_assert_str_eq( alias, "[fsfuse]Anonymous" );
    free( alias );

    ck_assert_int_eq( config_indexnode_advert_port( config ), 42444 );

    /* empty array */
    hosts = config_indexnode_hosts( config );
    fail_unless( hosts != NULL );
    fail_unless( hosts[0] == NULL );

    config_reader_delete( config );


    config_manager_add_from_file( test_isolate_file( strdup( "test_fsfuserc" ) ) );
    config_manager_add_from_file( test_isolate_file( strdup( "test_fsfuserc2" ) ) );
    config_manager_add_from_cmdline(
        1, 1, /* Set to same */
        1, 0, /* Set to different */
        0, 0  /* "unset" => argument ignored */
    );


    config = config_get_reader( );
    fail_unless( config != NULL, "config reader should be non-null" );

    /* Assert - check non-overridden values are not changed */
    ck_assert_int_eq( config_attr_mode_file( config ), 0444 );

    /* Assert - overridden by 1st file only */
    alias = config_alias( config );
    ck_assert_str_eq( alias, "test alias" );
    free( alias );

    /* Assert - overridden by 1st file, then again by 2nd */
    ck_assert_int_eq( config_indexnode_advert_port( config ), 22222 );

    /* Assert - overridden by 2nd file, then cmdline */
    ck_assert_int_eq( config_proc_fg( config ), 1 );
    ck_assert_int_eq( config_proc_singlethread( config ), 0 );
    ck_assert_int_eq( config_proc_debug( config ), 1 );

    hosts = config_indexnode_hosts( config );
    fail_unless( hosts != NULL );
    ck_assert_str_eq( hosts[0], "test host 21" );
    fail_unless(      hosts[1] == NULL );

    config_reader_delete( config );

    /* Teardown */
    config_singleton_delete( );
}
END_TEST

Suite *config_tests( void )
{
    Suite *s = suite_create( "config" );

    TCase *tc_stacking = tcase_create( "stacking" );
    tcase_add_test( tc_stacking, config_defaults_are_sane );
    tcase_add_test( tc_stacking, config_can_override_from_cmdline );
    tcase_add_test( tc_stacking, config_can_override_from_file );
    tcase_add_test( tc_stacking, config_can_override_from_several );

    suite_add_tcase( s, tc_stacking );

    return s;
}
