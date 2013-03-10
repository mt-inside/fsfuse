/*
 * Copyright (C) 2008-2013 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Class that listens for indexnodes and raises events when they are seen.
 */

#include "common.h"

#include <stdlib.h>
#include <unistd.h>

#include "indexnodes_listener_thread.h"


struct _indexnodes_listener_t
{
    pthread_t pthread_id;
    listener_thread_args_t *thread_args;
    int control_fd;
};


/* TODO:
 * indexnode "class" to be an opaque handle that stores handle to the thread
 * (s_listener_thread_info) and stashes it away in the user data in fuse_init.
 * indexnodes.c do indexnode state machine et al (have internal-only
 * indexnode_seen (aka reset state) fn.
 * make a fake indexnode for the console printer and integration test
 */
indexnodes_listener_t *indexnodes_listener_new (packet_received_cb_t packet_received_cb)
{
    indexnodes_listener_t *listener = malloc(sizeof(indexnodes_listener_t));
    listener->thread_args = malloc(sizeof(listener_thread_args_t));
    int pipe_fds[2];


    listener->thread_args->packet_received_cb = packet_received_cb;
    pipe(pipe_fds);
    listener->thread_args->control_fd = pipe_fds[0]; /* read end */
    listener->control_fd = pipe_fds[1]; /* write end */

    /* Start listening for broadcasting indexnodes */
    assert(
        !pthread_create(
            &(listener->pthread_id),
            NULL,
            &indexnodes_listen_main,
            listener->thread_args
        )
    );


    return listener;
}

void indexnodes_listener_delete (indexnodes_listener_t *listener)
{
    listener_control_codes_t msg = listener_control_codes_STOP;


    write(listener->control_fd, &msg, sizeof(msg));

    pthread_join(listener->pthread_id, NULL);

    free(listener->thread_args);
    free(listener);
}
