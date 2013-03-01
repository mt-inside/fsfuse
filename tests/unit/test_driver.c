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
    NOT_USED(argc);
    NOT_USED(argv);

    utils_init( );
    trace_init( );

    //hash_table_test();
    //http_test();
    test_indexnode( );
    test_proto_indexnode( );
    //string_buffer_test();
    //uri_test();
    //utils_test();

    trace_finalise( );
    utils_finalise( );

    printf( "OK.\n" );


    close( 0 );
    close( 1 );
    close( 2 );
    exit( EXIT_SUCCESS );
}
