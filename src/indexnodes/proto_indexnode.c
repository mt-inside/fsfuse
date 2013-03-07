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
#include "proto_indexnode_internal.h"

#include "curl_utils.h"
#include "utils.h"


const proto_indexnode_t *proto_indexnode_new( const char * const host, const char * const port )
{
    proto_indexnode_t *pin;


    assert(host); assert(*host);
    assert(port); assert(*port);


    pin = calloc( sizeof(proto_indexnode_t), 1 );

    pin->host = strdup( host );
    pin->port = strdup( port );


    return pin;
}

void proto_indexnode_delete( const proto_indexnode_t * const pin )
{
    free_const( pin->host );
    free_const( pin->port );

    free_const( pin );
}

const char *proto_indexnode_host( const proto_indexnode_t * const pin )
{
    return strdup( pin->host );
}

const char *proto_indexnode_port( const proto_indexnode_t * const pin )
{
    return strdup( pin->port );
}

/* TODO
 * Should be moved to the class that needs to use the URI
 * Should be decouped from CURL
 * Shouldn't over-estimate string length and sprintf - should use string_buffer
 */
const char *proto_indexnode_make_url (
    const proto_indexnode_t * const pin,
    const char * const path_prefix,
    const char * const resource
)
{
    char *fmt;
    char *resource_esc, *url;
    size_t len;
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
    len = strlen( pin->host ) + strlen( pin->port ) + strlen( path_prefix ) + strlen( resource_esc ) + strlen( fmt ) + 1;
    url = (char *)malloc( len * sizeof(*url) );
    sprintf( url, fmt, pin->host, pin->port, path_prefix, resource_esc );
    curl_free( resource_esc );

    curl_eh_delete( eh );


    return url;
}
