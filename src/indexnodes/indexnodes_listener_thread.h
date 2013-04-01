/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * Thread that listens for indexnode broadcats and raises events when they are
 * seen.
 */

#ifndef _INCLUDED_INDEXNODES_LISTENER_THREAD_H
#define _INCLUDED_INDEXNODES_LISTENER_THREAD_H

#include "common.h"

#include "indexnodes_internal.h"
#include "indexnodes_listener.h"


typedef enum
{
    listener_control_codes_NOT_USED,
    listener_control_codes_STOP
} listener_control_codes_t;

typedef struct
{
    new_indexnode_event_t packet_received_cb;
    void *packet_received_ctxt;
    int control_fd;
} listener_thread_args_t;


extern void *indexnodes_listen_main(void *args);

#endif /* _INCLUDED_INDEXNODES_LISTENER_THREAD_H */
