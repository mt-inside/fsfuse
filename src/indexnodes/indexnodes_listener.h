/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * Class to manage an indexnodes listener thread.
 */

#ifndef _INCLUDED_INDEXNODES_LISTENER_H
#define _INCLUDED_INDEXNODES_LISTENER_H

#include "common.h"


typedef void (*packet_received_cb_t) (
    const char * const host,
    const char * const port,
    const char * const version,
    const char * const id
);


extern void indexnodes_start_listening (packet_received_cb_t packet_received_cb);
extern void indexnodes_stop_listening (void);

#endif /* _INCLUDED_INDEXNODES_LISTENER_H */
