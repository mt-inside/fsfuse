/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * Class to manage an indexnodes listener thread.
 */

#ifndef _INCLUDED_INDEXNODES_LISTENER_H
#define _INCLUDED_INDEXNODES_LISTENER_H

#include "common.h"


typedef void (*packet_received_cb_t) (
    const void *ctxt,
    const char * const host,
    const char * const port,
    const char * const fs2protocol,
    const char * const id
);

typedef struct _indexnodes_listener_t indexnodes_listener_t;


extern indexnodes_listener_t *indexnodes_listener_new (packet_received_cb_t packet_received_cb, void *packet_received_ctxt);
extern void indexnodes_listener_delete (indexnodes_listener_t *listener);

#endif /* _INCLUDED_INDEXNODES_LISTENER_H */
