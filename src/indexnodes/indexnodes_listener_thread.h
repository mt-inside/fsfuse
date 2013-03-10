/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * Thread that listens for indexnode broadcats and raises events when they are
 * seen.
 */

#ifndef _INCLUDED_INDEXNODES_LISTENER_THREAD_H
#define _INCLUDED_INDEXNODES_LISTENER_THREAD_H

#include "common.h"


typedef void (*packet_received_cb_t) (
    const char * const host,
    const char * const port,
    const char * const version,
    const char * const id
);

typedef struct
{
    packet_received_cb_t packet_received_cb;
} indexnodes_listener_thread_info_t;


extern void *indexnodes_listen_main(void *args);

#endif /* _INCLUDED_INDEXNODES_LISTENER_THREAD_H */
