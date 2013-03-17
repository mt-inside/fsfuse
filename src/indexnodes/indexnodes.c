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
 */

/* TODO:
 * this assumes indexnodes live forever. Need to have the thread try to match
 * broadcasts against existing indexnodes. Unknown indexnodes are new. Known
 * indexnodes have their timeout put back to 0. Also needs to be a thread on a
 * timer that goes over all the indexnodes. If the timout has exceeded some
 * constant, change its state and remove it from the list, and dec ref count.
 * Anything that's still using it (they're ref-counted) sees it's in state dead,
 * and should error and return asap, deleting the indexnode as they do so.
 * For refcount to hit 0, the indexnode must not be owned by the list (assert
 * this) and so it can be free()d
 */

#include "common.h"

#include <stdlib.h>
#include <string.h>

#include "indexnodes.h"

#include "indexnodes_list_internal.h"
#include "indexnodes_listener.h"

#include "config.h"
#include "fetcher.h"
#include "queue.h"
#include "string_buffer.h"
#include "utils.h"


struct _indexnodes_t
{
    indexnodes_list_t *list;
    indexnodes_listener_t *listener;
    pthread_mutex_t lock; /* TODO: rwlock. TODO: shouldn't need locking */
};


static void load_indexnodes_from_config (indexnodes_list_t *list);
static const char *parse_version_cb (const proto_indexnode_t *in, const char *buf);
static void packet_received_cb (
    const void *ctxt,
    const char * const host,
    const char * const port,
    const char * const version,
    const char * const id
);


indexnodes_t *indexnodes_new (void)
{
    indexnodes_t *ins = malloc(sizeof(indexnodes_t));


    pthread_mutex_init(&(ins->lock), NULL);
    ins->list = indexnodes_list_new();

    load_indexnodes_from_config(ins->list);

    ins->listener = indexnodes_listener_new(&packet_received_cb, ins);


    return ins;
}

void indexnodes_delete (indexnodes_t *ins)
{
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
        const proto_indexnode_t *pin = proto_indexnode_new(strdup(host), strdup(port));

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

 /* TODO: If de's hold indexnodes then they either need to live for ever in state DEAD,
 * or be ref-counted and die. I suggest they're refcounted, and don't work when
 * in state dead. If ref-count hits 0 AND they're dead then they can die. de's
 * with DEAD indexnodes are dead themselves - when they discover that they
 * should act like they did a fetch and got a 404
 *
 * de's should make their own URIs and have their own fetch and list functions,
 * deferring to the right place. The ops should really only interact with them.
 * For now, still get the list of indexnodes for stat() at least.
 */
indexnodes_list_t *indexnodes_get (CALLER_DECL indexnodes_t *ins)
{
    indexnodes_list_t *list;


    pthread_mutex_lock(&(ins->lock));
    /* TODO: when listening is on a background thread and there is a state
     * machine, they shoudn't be removed, ever. Just filter for expired here
     * Expiring is a good idea because if they're expired they'll not get given
     * out any more and simply die when their last ref goes. Also means they can
     * be resurrected from the dead if a ping comes back.
     * Saying that, this list would still own them, so they should go from the
     * list when they reach a certain state, e.g. dead. THIS SHOULDN@T HAPPEN
     * HERE THOUGH. That means they really
     * would die when the last ref "in the wild" goes. They can be resurrected
     * up to that point, but after that a new one will be made, which is fine */
    ins->list = indexnodes_list_remove_expired(CALLER_PASS ins->list);
    list = indexnodes_list_copy(CALLER_PASS ins->list);
    pthread_mutex_unlock(&(ins->lock));


    return list;
}


static int parse_fs2protocol_version (const char *fs2protocol, const char **version)
{
    int rc = 1;
    char *v = malloc(strlen(fs2protocol) * sizeof(char));

    if (v &&
        sscanf(fs2protocol, "fs2protocol-%s", v) == 1)
    {
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
    indexnode_t *found_in, *new_in;
    const char *version, *new_id;


    if (!parse_fs2protocol_version(fs2protocol, &version))
    {
        /* TODO: It's IDs that identify indexnodes. find should take just an ID, not a
         * whole indexnodes! */
        new_in = indexnode_new(CALLER_INFO host, port, version, id);

        if (new_in)
        {
            pthread_mutex_lock(&(ins->lock));
            /* TODO: omfg this class shouldn't lock the list because the
             * list needs to be able to take copies when it knows it's not
             * being updated.
             * TODO: omfg the de-dup shouldn't be here - the list should do
             * it. Is this true? Should it acutally be here because it's a
             * fuinction of the management of indexnodes, not of a list of
             * them. Yes. Also getting a new packet for the same indexnode
             * uid should "Ping" the indexnode. I.e. find it in the list by
             * UID, indexnode_seen() if it exists, add if not. No. For
             * sanity now keep it as a class that does fuck all, except
             * maybe having an itterate-non-expired method.
             */
            new_id = indexnode_id(new_in);
            if (!(found_in = indexnodes_list_find(CALLER_INFO ins->list, new_id)))
            {
                /* If it's not been seen before, add it */
                indexnodes_list_add(ins->list, new_in);
            }
            else
            {
                /* TODO: reset timeout */
                indexnode_delete(CALLER_INFO new_in);
                indexnode_delete(CALLER_INFO found_in);
            }
            pthread_mutex_unlock(&(ins->lock));

            free_const(new_id);

            trace_info("Seen indexnode %s at %s:%s (version %s)\n", id, host, port, version);
        }

    }
    else
    {
        free_const(version); free_const(host); free_const(port); free_const(id);
    }
    free_const(fs2protocol);
}

static const char *parse_version_cb (const proto_indexnode_t *in, const char *fs2protocol)
{
    const char *version;

    assert(in);
    assert(fs2protocol); assert(*fs2protocol);

    parse_fs2protocol_version(fs2protocol, &version);

    return version;
}
