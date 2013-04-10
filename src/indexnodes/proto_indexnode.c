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

extern int proto_indexnode_get_info( proto_indexnode_t *pin,
                                     const char **protocol,
                                     const char **id )
{
    const char *url = proto_indexnode_make_url( pin, strdup( "browse" ), strdup( "" ) );

    return fetcher_get_indexnode_info( url, protocol, id );
}

/* TODO
 * Should be moved to the class that needs to use the URI
 * Should be decouped from CURL
 */
const char *proto_indexnode_make_url (
    const proto_indexnode_t *pin,
    const char *path_prefix,
    const char *resource
)
{
    string_buffer_t *sb = string_buffer_new( );
    char *fmt;
    char *resource_esc, *url;
    CURL *eh;


    assert(pin);
    assert(pin->host); assert(*pin->host);
    assert(pin->port); assert(*pin->port);

    eh = curl_eh_new( );


    if( is_ip4_address( pin->host ) || is_ip6_address( pin->host ))
    {
        fmt = "http://[%s]:%s/%s/%s";
    }
    else
    {
        fmt = "http://%s:%s/%s/%s";
    }


    /* we escape from resource + 1 and render the first '/' ourselves because
     * the indexnode insists on it being real */
    resource_esc = curl_easy_escape( eh, resource, 0 );
    string_buffer_printf( sb, url, fmt, proto_indexnode_host( pin ), proto_indexnode_port( pin ), path_prefix, resource_esc );
    url = string_buffer_commit( sb );

    free_const( resource );
    curl_free( resource_esc );
    curl_eh_delete( eh );


    return url;
}
