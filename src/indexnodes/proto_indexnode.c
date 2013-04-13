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

/* These header lines come in complete with their trailing new-lines.
 * HTTP/1.1 (RFC-2616) states that this sequence is \r\n
 *   (http://www.w3.org/Protocols/rfc2616/rfc2616-sec2.html#sec2.2)
 * From the libcurl API docs:
 *   "Do not assume that the header line is zero terminated!"
 */
static void match_header (const char *header, size_t len, const char *key, const char **value_out)
{
    size_t key_len, value_len;
    char *value;

    key_len = strlen(key);
    if (!strncasecmp(header, key, key_len))
    {
        value_len = len - key_len - 2; /* -2 for line-end */
        value = malloc(value_len + 1); /* +1 for terminating \0 */
        strncpy(value, header + key_len, value_len);
        value[value_len] = '\0';
        *value_out = value;
    }
}

static size_t header_cb (void *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t len = size * nmemb;
    indexnode_info_t *info = (indexnode_info_t *)stream;
    char *header = (char *)ptr;


    match_header(header, len, fs2_version_header_key, &(info->protocol));
    match_header(header, len, fs2_indexnode_uid_header_key, &(info->id));


    return len;
}


int proto_indexnode_get_info( proto_indexnode_t *pin,
                              const char **protocol,
                              const char **id )
{
    int rc;
    indexnode_info_t *info =
        malloc(sizeof(indexnode_info_t));


    rc = fetch(
        proto_indexnode_make_url( pin, strdup( "browse" ), strdup( "" ) ),
        (curl_write_callback)&header_cb,
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
