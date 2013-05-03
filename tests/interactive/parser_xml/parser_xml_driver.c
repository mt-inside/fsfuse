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

#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#include "parser/parser_xml.h"


static void print_cb( void *ctxt, parser_xml_event_t event, char *text )
{
    NOT_USED(ctxt);

    switch( event )
    {
        case parser_xml_event_TAG_START:
            printf( "<%s>\n", text );
            break;
        case parser_xml_event_TEXT:
            printf( "\"%s\"\n", text );
            break;
        case parser_xml_event_TAG_END:
            printf( "</%s>\n", text );
            break;
        default:
            assert( 0 );
    }
}

static void feed_file( const char *name, parser_xml_t *xml )
{
    //const char *path = test_isolate_file( name );
    int fd = open( name, O_RDONLY );
    ssize_t readed;
    char buf[ 16 ];

    while( (readed = read( fd, buf, sizeof(buf) )) )
    {
        parser_xml_consume( xml, buf, readed );
    }

    close( fd );
}

int main( int argc, char **argv )
{
    NOT_USED(argc);
    NOT_USED(argv);


    parser_xml_t *xml = parser_xml_new( &print_cb, NULL );

    feed_file( strdup( "stats.xml" ), xml );

    parser_xml_delete( xml );


    return 0;
}
