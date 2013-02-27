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

#include "common.h"

#include <stdlib.h>
#include <string.h>

#include "indexnode.h"
#include "proto_indexnode_internal.h"

#include "config.h"
#include "fetcher.h"
#include "utils.h"


struct _indexnode_t
{
    proto_indexnode_t pin;

    pthread_mutex_t *lock;
    unsigned ref_count;

    char *version;
    char *id;

    time_t last_seen;
};

#define BASE_CLASS(in) ((proto_indexnode_t *)in)


static int check_version (const char * const version)
{
    return (compare_dotted_version(PROTO_MINIMUM, version) < 0 ||
            compare_dotted_version(version, PROTO_MAXIMUM) < 0);
}

/* TODO: state machine for active, missed 1 ping, etc */
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

        BASE_CLASS(in)->host = strdup(host);
        BASE_CLASS(in)->port = strdup(port);
        in->version = strdup(version);
        in->id      = strdup(id);

        indexnode_seen(in);
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
    indexnode_t *in = NULL;


    assert(pin);
    assert(version); assert(*version);

    if (check_version(version))
    {
        in = calloc(sizeof(indexnode_t), 1);

        BASE_CLASS(in)->host = proto_indexnode_host( pin );
        BASE_CLASS(in)->port = proto_indexnode_port( pin );
        in->version = strdup(version);

        indexnode_seen(in);
    }
    else
    {
        trace_warn("Ignoring indexnode of version %s, only versions %s <= x <= %s are supported\n",
                   version, PROTO_MINIMUM, PROTO_MAXIMUM);
    }

    proto_indexnode_delete(pin);


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

        free_const(BASE_CLASS(in)->host);
        free_const(BASE_CLASS(in)->port);
        free(in->version);
        if (in->id) free(in->id);

        free(in);
    }
}

char *indexnode_get_host (indexnode_t *in)
{
    return proto_indexnode_host( BASE_CLASS(in) );
}

char *indexnode_get_port (indexnode_t *in)
{
    return proto_indexnode_port( BASE_CLASS(in) );
}

char *indexnode_get_version (indexnode_t *in)
{
    return strdup( in->version );
}

char *indexnode_get_id (indexnode_t *in)
{
    return strdup( in->id );
}

/* API to make URLs ========================================================= */

char *indexnode_make_url (
    indexnode_t *in,
    const char * const path_prefix,
    const char * const resource
)
{
    return proto_indexnode_make_url( BASE_CLASS(in), path_prefix, resource );
}

void indexnode_seen (indexnode_t *in)
{
    in->last_seen = time(NULL);
}

int indexnode_still_valid (indexnode_t *in)
{
    return (time(NULL) - in->last_seen) < config_indexnode_timeout;
}
