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


typedef struct _listener_thread_info_t
{
    pthread_t pthread_id;
    listener_thread_args_t *thread_args;
    int control_fd;
} listener_thread_info_t;


static listener_thread_info_t *s_listener_thread_info;


/* TODO:
 * have indexnodes be a class and have it call start_listening when it's
 * new'd, stop when it's deleted.
 * indexnode "class" to be an opaque handle that stores handle to the thread
 * (s_listener_thread_info) and stashes it away in the user data in fuse_init.
 * indexnodes.c do indexnode state machine et al (have internal-only
 * indexnode_seen (aka reset state) fn.
 * make a fake indexnode for the console printer and integration test
 */
void indexnodes_start_listening (packet_received_cb_t packet_received_cb)
{
    s_listener_thread_info = malloc(sizeof(listener_thread_info_t));
    s_listener_thread_info->thread_args = malloc(sizeof(listener_thread_args_t));
    int pipe_fds[2];


    s_listener_thread_info->thread_args->packet_received_cb = packet_received_cb;
    pipe(pipe_fds);
    s_listener_thread_info->thread_args->control_fd = pipe_fds[0]; /* read end */
    s_listener_thread_info->control_fd = pipe_fds[1]; /* write end */

    /* Start listening for broadcasting indexnodes */
    assert(
        !pthread_create(
            &(s_listener_thread_info->pthread_id),
            NULL,
            &indexnodes_listen_main,
            s_listener_thread_info->thread_args
        )
    );
}

void indexnodes_stop_listening (void)
{
    listener_control_codes_t msg = listener_control_codes_STOP;


    write(s_listener_thread_info->control_fd, &msg, sizeof(msg));

    pthread_join(s_listener_thread_info->pthread_id, NULL);

    free(s_listener_thread_info->thread_args);
    free(s_listener_thread_info);
}
