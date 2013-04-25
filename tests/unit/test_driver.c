/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Unit tests "driver" - provides the main() symbol in unit test builds.
 */

#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "tests.h"

#include "utils.h"


int main (int argc, char **argv)
{
    int num_failed;


    utils_init( );
    trace_init( );

    SRunner *r = srunner_create( NULL );
    srunner_add_suite( r, config_tests( ) );
    srunner_add_suite( r, indexnode_tests( ) );
    srunner_add_suite( r, indexnodes_list_tests( ) );
    srunner_add_suite( r, proto_indexnode_tests( ) );
    srunner_add_suite( r, ref_count_tests( ) );
    srunner_add_suite( r, string_buffer_tests( ) );

    if( argc == 2 && !strcmp( argv[1], "-n" ) ) srunner_set_fork_status( r, CK_NOFORK );

    srunner_run_all( r, CK_NORMAL );

    num_failed = srunner_ntests_failed( r );

    srunner_free( r );

    trace_finalise( );
    utils_finalise( );

    close( 0 );
    close( 1 );
    close( 2 );

    return (num_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;

    //http_test();
    //uri_test();
    //utils_test();
}

extern const char *const testdata_path;
char *test_isolate_file( char *name )
{
    return path_combine( strdup( testdata_path ), name );
}
