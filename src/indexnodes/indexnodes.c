/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * The collection of known indexnodes.
 * This module spanws a thread that listens for indexnode broadcats and
 * maintains a list.
 * There is also a periodic alarm that causes dead indexnodes to be removed
 * from the list, causing the list to give up its ownership of them (rather than
 * doing it lazily on list fectch which would use more memory if the list was
 * never fecthed but lots of indexnodes were seen).
 */

#include "common.h"

#include <stdlib.h>
#include <string.h>

#include "indexnodes.h"

#include "indexnodes_list_internal.h"
#include "indexnodes_listener.h"

#include "alarm_simple.h"
#include "config.h"
#include "fetcher.h"
#include "queue.h"
#include "string_buffer.h"
#include "utils.h"


struct _indexnodes_t
{
    indexnodes_list_t *list;
    indexnodes_listener_t *listener;
    alarm_t *expire_alarm;
    pthread_mutex_t lock; /* TODO: rwlock. */
};


static void load_indexnodes_from_config (indexnodes_list_t *list);
static const char *parse_version_cb (proto_indexnode_t *in, const char *buf);
static void packet_received_cb (
    const void *ctxt,
    const char * const host,
    const char * const port,
    const char * const version,
    const char * const id
);
static void expire_alarm (void *ctxt);


indexnodes_t *indexnodes_new (void)
{
    indexnodes_t *ins = malloc(sizeof(indexnodes_t));


    pthread_mutex_init(&(ins->lock), NULL);
    ins->list = indexnodes_list_new();

    load_indexnodes_from_config(ins->list);

    ins->listener = indexnodes_listener_new(&packet_received_cb, ins);
    ins->expire_alarm = alarm_new(1, &expire_alarm, ins);


    return ins;
}

void indexnodes_delete (indexnodes_t *ins)
{
    alarm_delete(ins->expire_alarm);
    /* TODO: assert the thread state == stopped */
    /* TODO: should we do this, or should the thread bump their statemachine to
     * dead, let them die, then just assert they're dead here? */
    indexnodes_listener_delete(ins->listener);

    /* TODO: delete static indexnodes? - does the list own them? do we? */

    indexnodes_list_delete(ins->list);
    pthread_mutex_destroy(&(ins->lock));

    free(ins);
}

static void load_indexnodes_from_config (indexnodes_list_t *list)
{
    int i = 0;
    const char *host, *port, *version;
    string_buffer_t *id_buffer;


    while ((host = config_indexnode_hosts[i]) &&
           (port = config_indexnode_ports[i]))
    {
        /* TODO: No need to strdup these when config is a real class with real
         * getters that return copies */
        proto_indexnode_t *pin = proto_indexnode_new(strdup(host), strdup(port));

        /* TODO: should do this async, in parallel, so that it happens as fast
         * as possible and that we can get on with other stuff straight away.
         * The following logic would have to move to a callback. */
        /* TODO: what happens if this fails? */
        /* TODO: "get version and id headers" or soemthign */
        /* TODO: should just return fs2protocol header string and we'll parse it
         * ourselves */
        version = fetcher_get_indexnode_version(pin, &parse_version_cb); /* blocks */
        id_buffer = string_buffer_new();
        string_buffer_printf(id_buffer, "static-indexnode-%d", i);

        indexnode_t *in = indexnode_from_proto(CALLER_INFO pin, version, string_buffer_peek(id_buffer));
        if (in)
        {
            /* TODO: can now get the uid of an indexnode from an HTTP header -
             * do this. For now it's just added assuming there isn't a duplicate */
            /* TODO: raise the event. Does this mean we don't need proto
             * indexnodd? I don't thing so - it'll be a useful ctxt when we do
             * the get_version_and_id in parallel */
            indexnodes_list_add(list, in);

            /* TODO: free these, yo */
            trace_info(
                "Static index node configured at %s:%s, version %s, id %s\n",
                indexnode_host(in),
                indexnode_port(in),
                indexnode_version(in),
                indexnode_id(in));
        }

        string_buffer_delete(id_buffer);
        i++;
    }
}

/* de's should make their own URIs and have their own fetch and list functions,
 * deferring to the right place. The ops should really only interact with them.
 * For now, still get the list of indexnodes for stat() at least.
 */
indexnodes_list_t *indexnodes_get (CALLER_DECL indexnodes_t *ins)
{
    indexnodes_list_t *list;


    pthread_mutex_lock(&(ins->lock));
    list = indexnodes_list_copy(CALLER_PASS ins->list);
    pthread_mutex_unlock(&(ins->lock));


    return list;
}


static int parse_fs2protocol_version (const char *fs2protocol, const char **version)
{
    int rc = 1;
    const char *prefix = "fs2protocol-";
    const size_t prefix_len = strlen(prefix);

    if (!strncmp(fs2protocol, prefix, prefix_len))
    {
        char *v = malloc((strlen(fs2protocol) - prefix_len + 1) * sizeof(char));

        assert(sscanf(fs2protocol, "fs2protocol-%s", v) == 1);

        *version = v;
        rc = 0;
    }

    return rc;
}

static void packet_received_cb (
    const void *ctxt,
    const char * const host,
    const char * const port,
    const char * const fs2protocol,
    const char * const id
)
{
    indexnodes_t *ins = (indexnodes_t *)ctxt;
    indexnode_t *found_in, *new_in = NULL;
    const char *version;


    if (!parse_fs2protocol_version(fs2protocol, &version))
    {
        trace_info("Seen indexnode %s at %s:%s (version %s)\n", id, host, port, version);

        if ((found_in = indexnodes_list_find(CALLER_INFO ins->list, id)))
        {
            indexnode_seen(found_in);

            indexnode_delete(CALLER_INFO found_in);
        }
        else
        {
            new_in = indexnode_new(CALLER_INFO host, port, version, id);
            if (new_in)
            {
                pthread_mutex_lock(&(ins->lock));

                /* If it's not been seen before, add it */
                indexnodes_list_add(ins->list, new_in);

                pthread_mutex_unlock(&(ins->lock));
            }
        }
    }

    free_const(fs2protocol);
    if (!new_in)
    {
        free_const(host); free_const(port); free_const(version); free_const(id);
    }
}

static const char *parse_version_cb (proto_indexnode_t *in, const char *fs2protocol)
{
    const char *version;

    assert(in);
    assert(fs2protocol); assert(*fs2protocol);

    parse_fs2protocol_version(fs2protocol, &version);

    return version;
}

static void expire_alarm (void *ctxt)
{
    indexnodes_t *ins = (indexnodes_t *)ctxt;


    pthread_mutex_lock(&(ins->lock));

    ins->list = indexnodes_list_remove_expired(CALLER_INFO ins->list);

    pthread_mutex_unlock(&(ins->lock));
}
