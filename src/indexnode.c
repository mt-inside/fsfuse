/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Indexnode class.
 */

 /* NB: this file relies on _POSIX_SOURCE to get NI_MAX[HOST|etc], even though
  * the man page and netdb.h seem to say it really should be _BSD_SOURCE */

#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "common.h"
#include "config.h"
#include "curl_utils.h"
#include "indexnode.h"
#include "fetcher.h"
#include "utils.h"


struct _indexnode_t
{
    char *host;
    char *port;
    char *version;
    char *id;
};


/* TODO: state machine */
indexnode_t *indexnode_new_static (char *host, char *port)
{
    indexnode_t *in;


    assert(host); assert(*host);
    assert(port); assert(*port);


    in = calloc(sizeof(indexnode_t), 1);
    in->host = strdup(host);
    in->port = strdup(port);


    return in;
}

indexnode_t *indexnode_new (char *host, char *port, char *version, char *id)
{
    indexnode_t *in;


    assert(version); assert(*version);
    assert(id);      assert(*id);


    in = indexnode_new(host, port);
    in->version = strdup(version);
    in->id      = strdup(id     );


    return in;
}

void indexnode_delete (indexnode_t *in)
{
    free(in->host);
    free(in->port);
    if (in->version) free(in->version);
    if (in->id)      free(in->id);

    free(in);
}

char *indexnode_get_host (indexnode_t *in)
{
    return in->host;
}

char *indexnode_get_port (indexnode_t *in)
{
    return in->port;
}

/* TODO: such is my hatred for mutable state that this has to go.
 * Either indexnode_make_url has to take a host and a port, not an indexnode, or
 * take a proto_indexnode (base class of indexnode), from which a full indexnode
 * can be constructed when we know its version */
void indexnode_set_version (indexnode_t *in, char *version)
{
    in->version = strdup(version);
}
char *indexnode_get_version (indexnode_t *in)
{
    return in->version;
}

char *indexnode_get_id (indexnode_t *in)
{
    return in->id;
}

/* API to make URLs ========================================================= */

char *indexnode_make_url (
    indexnode_t *in,
    const char * const path_prefix,
    const char * const resource
)
{
    char *host = indexnode_get_host(in), *port = indexnode_get_port(in);
    char *fmt;
    char *resource_esc, *url;
    size_t len;
    CURL *eh = curl_eh_new();


    if (is_ip4_address(host) || is_ip6_address(host))
    {
        fmt = "http://[%s]:%s/%s/%s";
    }
    else
    {
        fmt = "http://%s:%s/%s/%s";
    }


    /* we escape from resource + 1 and render the first '/' ourselves because
     * the indexnode insists on it being real */
    resource_esc = curl_easy_escape(eh, resource, 0);
    len = strlen(host) + strlen(port) + strlen(path_prefix) + strlen(resource_esc) + strlen(fmt) + 1;
    url = (char *)malloc(len * sizeof(*url));
    sprintf(url, fmt, indexnode_get_host(in), indexnode_get_port(in), path_prefix, resource_esc);
    curl_free(resource_esc);

    curl_eh_delete(eh);


    return url;
}
