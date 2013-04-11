/*
 * Copyright (C) 2008-2013 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Class to manage the set of statically-configured indexnodes.
 */

#include "common.h"

#include <stdlib.h>
#include <string.h>

#include "indexnodes_statics_manager.h"

#include "alarm_simple.h"
#include "config.h"
#include "linked_list.h"
#include "proto_indexnode.h"


typedef struct
{
    indexnodes_statics_manager_t *mgr;
    proto_indexnode_t *pin;
} mgr_pin_pair_t;

LINKED_LIST_ENTRY_T(alarms_list_t, alarm_t *);

struct _indexnodes_statics_manager_t
{
    LINKED_LIST_T(alarms_list_t) alarms;
    new_indexnode_event_t cb;
    void *cb_ctxt;
};


static void load_indexnodes_from_config (indexnodes_statics_manager_t *mgr);


indexnodes_statics_manager_t *indexnodes_statics_manager_new(
    new_indexnode_event_t cb,
    void *ctxt
)
{
    indexnodes_statics_manager_t *mgr = malloc( sizeof(*mgr) );


    mgr->cb = cb;
    mgr->cb_ctxt = ctxt;
    mgr->alarms = LINKED_LIST_INIT;

    load_indexnodes_from_config(mgr);

    return mgr;
}

void indexnodes_statics_manager_delete(
    indexnodes_statics_manager_t *mgr
)
{
    LINKED_LIST_DELETE(mgr->alarms, alarm_delete);

    free( mgr );
}

/* TODO: What thread does this happen on? What does that mean? */
static void ping_indexnode( void *ctxt )
{
    mgr_pin_pair_t *pair = (mgr_pin_pair_t *)ctxt;
    proto_indexnode_t *pin = pair->pin;
    indexnodes_statics_manager_t *mgr = pair->mgr;
    const char *protocol, *id;


    /* TODO: what happens if this fails? */
    if (proto_indexnode_get_info(pin, &protocol, &id)) /* blocks */
    {
        mgr->cb(
            mgr->cb_ctxt,
            proto_indexnode_host(pin),
            proto_indexnode_port(pin),
            protocol,
            id
        );
    }
}

static void load_indexnodes_from_config (indexnodes_statics_manager_t *mgr)
{
    alarm_t *alarm;
    mgr_pin_pair_t *pair;
    const char *host, *port;
    int i = 0;


    while ((host = config_indexnode_hosts[i]) &&
           (port = config_indexnode_ports[i]))
    {
        /* TODO: No need to strdup these when config is a real class with real
         * getters that return copies */
        proto_indexnode_t *pin = proto_indexnode_new(strdup(host), strdup(port));
        /* FIXME: pairs and protos need freeing. Make the list be of pairs
         * instead */
        pair = malloc( sizeof(mgr_pin_pair_t) );
        pair->mgr = mgr;
        pair->pin = pin;

        alarm = alarm_new( config_indexnode_timeout / 2, ping_indexnode, pair );
        LINKED_LIST_ADD(mgr->alarms, alarm);

        i++;
    }
}
