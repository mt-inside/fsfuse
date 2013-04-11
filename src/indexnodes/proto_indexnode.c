/*
 * Copyright (C) 2008-2013 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Proto-indexnode class.
 */

#include "common.h"

#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "proto_indexnode.h"

#include "curl_utils.h"
#include "fetcher.h"
#include "string_buffer.h"
#include "utils.h"


proto_indexnode_t *proto_indexnode_new( const char *host, const char *port )
{
    proto_indexnode_t *pin;


    pin = calloc( sizeof(proto_indexnode_t), 1 );
    proto_indexnode_init( pin, host, port );


    return pin;
}

void proto_indexnode_init(
    proto_indexnode_t *pin,
    const char *host,
    const char *port
)
{
    assert(host); assert(*host);
    assert(port); assert(*port);

    pin->host = host;
    pin->port = port;
}

void proto_indexnode_delete( proto_indexnode_t *pin )
{
    proto_indexnode_teardown( pin );

    free( pin );
}

void proto_indexnode_teardown( proto_indexnode_t *pin )
{
    free_const( pin->host );
    free_const( pin->port );
}

const char *proto_indexnode_host( const proto_indexnode_t *pin )
{
    return strdup( pin->host );
}

const char *proto_indexnode_port( const proto_indexnode_t *pin )
{
    return strdup( pin->port );
}

int proto_indexnode_get_info( proto_indexnode_t *pin,
                              const char **protocol,
                              const char **id )
{
    /* TODO: this should be more generic. This (pin) class should be parsing the
     * responses */
    return fetcher_get_indexnode_info(
        proto_indexnode_make_url( pin, strdup( "browse" ), strdup( "" ) ),
        protocol,
        id
    );
}

static const char *make_path (
    const char *path_prefix,
    const char *resource
)
{
    string_buffer_t *sb = string_buffer_new( );
    char *path;


    assert(path_prefix); assert(*path_prefix);
    assert(resource);

    string_buffer_printf( sb, "%s/%s", path_prefix, fetcher_escape_for_http( resource ) );
    path = string_buffer_commit( sb );


    return path;
}

const char *proto_indexnode_make_url(
    proto_indexnode_t *pin,
    const char *path_prefix,
    const char *resource
)
{
    /* I think it's OK for the indexnode to know that indexnodes are accessed
     * over HTTP and what the URI format is */
    return fetcher_make_http_url(
        proto_indexnode_host( pin ),
        proto_indexnode_port( pin ),
        make_path( path_prefix, resource )
    );
}
