/*
 * Copyright (C) 2008-2013 Matthew Turner.
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

#include "ref_count.h"
#include "config.h"
#include "fetcher.h"
#include "utils.h"


TRACE_DEFINE(indexnode)


struct _indexnode_t
{
    proto_indexnode_t pin;

    ref_count_t *ref_count;

    const char *version;
    const char *id;

    time_t last_seen;
};

#define BASE_CLASS(in) ((proto_indexnode_t *)in)


static int check_version( const char * const version )
{
    return( compare_dotted_version(PROTO_MINIMUM, version) < 0 ||
            compare_dotted_version(version, PROTO_MAXIMUM) < 0 );
}

/* TODO: state machine for active, missed 1 ping, etc */
indexnode_t *indexnode_new(
    CALLER_DECL
    const char * const host,
    const char * const port,
    const char * const version,
    const char * const id
)
{
    indexnode_t *in = NULL;


    assert(version); assert(*version);
    assert(id);      assert(*id);

    if( check_version( version ) )
    {
        in = calloc( sizeof(indexnode_t), 1 );

        proto_indexnode_init( BASE_CLASS(in), host, port );

        in->ref_count = ref_count_new( );

        in->version          = strdup( version );
        in->id               = strdup( id );

        indexnode_seen( in );

        indexnode_trace(
            "[indexnode @%p id %s] new (" CALLER_FORMAT ") ref %u\n",
            in, in->id, CALLER_PASS 1
        );
    }
    else
    {
        trace_warn( "Ignoring indexnode of version %s, only versions %s <= x <= %s are supported\n",
                    version, PROTO_MINIMUM, PROTO_MAXIMUM );
    }


    return in;

}

indexnode_t *indexnode_from_proto(
    CALLER_DECL
    const proto_indexnode_t * const pin,
    const char * const version,
    const char * const id
)
{
    indexnode_t *in;
    const char *host = proto_indexnode_host( pin );
    const char *port = proto_indexnode_port( pin );


    in = indexnode_new(
        CALLER_PASS
        host,
        port,
        version,
        id
    );


    free_const( host );
    free_const( port );
    proto_indexnode_delete( pin );


    return in;
}

indexnode_t * indexnode_post( CALLER_DECL indexnode_t * const in )
{
    unsigned refc = ref_count_inc( in->ref_count );

    indexnode_trace("[indexnode @%p id %s] post (" CALLER_FORMAT ") ref %u\n",
                   in, in->id, CALLER_PASS refc);


    return in;
}

void indexnode_delete( CALLER_DECL indexnode_t * const in )
{
    unsigned refc = ref_count_dec( in->ref_count );

    /* This should go in the dec method, which should take the logger (which can
     * be queried for class_name, or know how to log a delete). This way it can
     * be done under the lock (at least copy the data out under the lock - file
     * IO under the lock is not clever) */
    indexnode_trace("[indexnode @%p] delete (" CALLER_FORMAT ") ref %u\n",
                     in, CALLER_PASS refc);
    indexnode_trace_indent();

    if (!refc)
    {
        indexnode_trace("refcount == 0 => free()ing\n");

        ref_count_delete( in->ref_count );

        proto_indexnode_teardown( BASE_CLASS(in) );
        free_const( in->version );
        free_const( in->id );

        free( in );
    }

    indexnode_trace_dedent();
}

const char *indexnode_host( const indexnode_t * const in )
{
    return proto_indexnode_host( BASE_CLASS(in) );
}

const char *indexnode_port( const indexnode_t * const in )
{
    return proto_indexnode_port( BASE_CLASS(in) );
}

const char *indexnode_version( const indexnode_t * const in )
{
    return strdup( in->version );
}

const char *indexnode_id( const indexnode_t * const in )
{
    return strdup( in->id );
}

/* API to make URLs ========================================================= */

/* TODO: move me! */
const char *indexnode_make_url(
    const indexnode_t * const in,
    const char * const path_prefix,
    const char * const resource
)
{
    return proto_indexnode_make_url( BASE_CLASS(in), path_prefix, resource );
}

void indexnode_seen( indexnode_t * const in )
{
    in->last_seen = time( NULL );
}

int indexnode_still_valid( const indexnode_t * const in )
{
    return ( time( NULL ) - in->last_seen ) < config_indexnode_timeout;
}