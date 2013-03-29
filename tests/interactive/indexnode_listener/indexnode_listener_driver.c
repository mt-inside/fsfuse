/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Interactive indexnode listener test "driver" - provides the main() symbol,
 * which prints all the known indexnodes in a loop.
 */

#include "common.h"

#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#include "indexnodes.h"
#include "utils.h"


static volatile int s_exiting = 0;


static void sigint_handler( int signum )
{
    NOT_USED(signum);

    s_exiting = 1;
}

static void print_indexnode( indexnode_t *in )
{
    const char *host = indexnode_host(in),
               *port = indexnode_port(in),
               *id = indexnode_version(in),
               *version = indexnode_version(in);

    printf( "[indexnode %p] %s @ %s:%s (version %s)\n", (void *)in, id, host, port, version );

    free_const( host );
    free_const( port );
    free_const( id );
    free_const( version );
}

int main( int argc, char **argv )
{
    indexnodes_t *ins;
    indexnodes_list_t *list;
    indexnodes_iterator_t *iter;
    indexnode_t *in;


    utils_init( );
    trace_init( );
    indexnode_trace_on( );

    NOT_USED(argc);
    NOT_USED(argv);

    signal( SIGINT, &sigint_handler );

    ins = indexnodes_new();

    while( !s_exiting )
    {
        printf("loop\n");

        list = indexnodes_get( CALLER_INFO ins );
        for( iter = indexnodes_iterator_begin( list );
             !indexnodes_iterator_end( iter );
             iter = indexnodes_iterator_next( iter ) )
        {
            in = indexnodes_iterator_current( iter );

            print_indexnode( in );

            indexnode_delete( CALLER_INFO in );
        }
        indexnodes_iterator_delete( iter );
        indexnodes_list_delete( list );

        sleep( 1 );
    }

    indexnodes_delete( ins );

    trace_finalise( );
    utils_finalise( );


    return 0;
}
