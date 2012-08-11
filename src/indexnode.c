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
    pthread_mutex_t    *lock;
    unsigned ref_count;

    char *host;
    char *port;
    char *version;
    char *id;
};

/* essentially Pair<host, port>. Here because nature abhors a mutable type */
struct _proto_indexnode_t
{
    char *host;
    char *port;
};


proto_indexnode_t *proto_indexnode_new (char *host, char *port)
{
    proto_indexnode_t *pin;


    assert(host); assert(*host);
    assert(port); assert(*port);


    pin = calloc(sizeof(proto_indexnode_t), 1);

    pin->host = strdup(host);
    pin->port = strdup(port);


    return pin;
}

static int check_version (const char * const version)
{
    return (compare_dotted_version(PROTO_MINIMUM, version) < 0 ||
            compare_dotted_version(version, PROTO_MAXIMUM) < 0);
}

/* TODO: state machine */
indexnode_t *indexnode_new (char *host, char *port, char *version, char *id)
{
    indexnode_t *in = NULL;


    assert(host);    assert(*host);
    assert(port);    assert(*port);
    assert(version); assert(*version);
    assert(id);      assert(*id);

    if (check_version(version))
    {
        in = calloc(sizeof(indexnode_t), 1);

        in->host    = strdup(host);
        in->port    = strdup(port);
        in->version = strdup(version);
        in->id      = strdup(id);
    }
    else
    {
        trace_warn("Ignoring indexnode of version %s, only versions %s <= x <= %s are supported\n",
                   version, PROTO_MINIMUM, PROTO_MAXIMUM);
    }


    return in;

}

/* TODO: when in inherits pin, these ctors can be combined, e.g only
 * check_version in base ctor */
indexnode_t *indexnode_from_proto (proto_indexnode_t *pin, char *version)
{
    indexnode_t *in;


    assert(pin);
    assert(version); assert(*version);

    if (check_version(version))
    {
        in = calloc(sizeof(indexnode_t), 1);

        /* usurp ownership of these. Slightly nasty, but it's all in one class. */
        in->host    = pin->host;
        in->port    = pin->port;
        in->version = strdup(version);
    }
    else
    {
        trace_warn("Ignoring indexnode of version %s, only versions %s <= x <= %s are supported\n",
                   version, PROTO_MINIMUM, PROTO_MAXIMUM);
    }

    free(pin);


    return in;
}

indexnode_t *indexnode_post (indexnode_t *in)
{
    pthread_mutex_lock(in->lock);
    assert(in->ref_count);
    ++in->ref_count;
    pthread_mutex_unlock(in->lock);


    return in;
}

void indexnode_delete (indexnode_t *in)
{
    unsigned refc;


    pthread_mutex_lock(in->lock);
    assert(in->ref_count);
    refc = --in->ref_count;
    pthread_mutex_unlock(in->lock);

    if (!refc)
    {
        pthread_mutex_destroy(in->lock);
        free(in->lock);

        free(in->host);
        free(in->port);
        free(in->version);
        if (in->id) free(in->id);

        free(in);
    }
}

char *indexnode_get_host (indexnode_t *in)
{
    return in->host;
}

char *indexnode_get_port (indexnode_t *in)
{
    return in->port;
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

static char *make_url_inner (
    const char * const host,
    const char * const port,
    const char * const path_prefix,
    const char * const resource
);

/* Neccessary to get an static indexnode's version before making a full one */
/* TODO: have in inherit pin so in can be downcast and there can be just one
 * function */
char *proto_indexnode_make_url (
    proto_indexnode_t *pin,
    const char * const path_prefix,
    const char * const resource
)
{
    return make_url_inner(pin->host, pin->port, path_prefix, resource);
}

char *indexnode_make_url (
    indexnode_t *in,
    const char * const path_prefix,
    const char * const resource
)
{
    return make_url_inner(in->host, in->port, path_prefix, resource);
}

static char *make_url_inner (
    const char * const host,
    const char * const port,
    const char * const path_prefix,
    const char * const resource
)
{
    char *fmt;
    char *resource_esc, *url;
    size_t len;
    CURL *eh;


    assert(host); assert(*host);
    assert(port); assert(*port);

    eh = curl_eh_new();


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
    sprintf(url, fmt, host, port, path_prefix, resource_esc);
    curl_free(resource_esc);

    curl_eh_delete(eh);


    return url;
}
