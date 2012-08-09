/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * FUSE filesystem initialisation function.
 */

#include "common.h"

#include <fuse/fuse_lowlevel.h>
#include <errno.h>

#include "fsfuse_ops/fsfuse_ops.h"
#include "trace.h"


void fsfuse_init (void *userdata, struct fuse_conn_info *conn)
{
    NOT_USED(userdata);
    NOT_USED(conn);

    method_trace("fsfuse_init()\n");
    method_trace_indent();

    method_trace(
        "proto_major: %u, proto_minor: %u, "
        "async_read: %u, max_write: %u, max_readahead: %u\n",
        conn->proto_major,
        conn->proto_minor,
        conn->async_read,
        conn->max_write,
        conn->max_readahead
    );

    method_trace_dedent();


    /* No reply */
}
