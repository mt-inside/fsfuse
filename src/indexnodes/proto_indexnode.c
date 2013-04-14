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

#include "proto_indexnode.h"

#include "fetcher.h"
#include "fs2_constants.h"
#include "string_buffer.h"
#include "utils.h"


typedef struct
{
    const char *protocol;
    const char *id;
} indexnode_info_t;


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

static int header_cb (void *ctxt, const char *key, const char *value)
{
    indexnode_info_t *info = (indexnode_info_t *)ctxt;


    if( !strcasecmp( key, fs2_version_header_key ) )
    {
        info->protocol = value;
    }
    else if( !strcasecmp( key, fs2_indexnode_uid_header_key ) )
    {
        info->id = value;
    }


    return 0;
}


int proto_indexnode_get_info( proto_indexnode_t *pin,
                              const char **protocol,
                              const char **id )
{
    const char *url = proto_indexnode_make_url( pin, strdup( "browse" ), strdup( "" ) );
    indexnode_info_t *info = malloc(sizeof(indexnode_info_t));
    int rc;


    rc = fetch(
        url,
        (fetcher_header_cb_t)&header_cb,
        info,
        NULL,
        NULL,
        1,
        NULL
    );


    *protocol = info->protocol;
    *id = info->id;
    free(info);


    return !rc &&
           *protocol && **protocol &&
           *id       && **id;
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
