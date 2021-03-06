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

#include <errno.h>

#include "fuse_methods.h"
#include "indexnodes.h"
#include "trace.h"

/* "Miscellaneous threads should be started from the init() method. Threads
 *  started before fuse_main() will exit when the process goes into the
 *  background."
 */

void fsfuse_init (void *userdata, struct fuse_conn_info *conn)
{
    fsfuse_ctxt_t *ctxt = (fsfuse_ctxt_t *)userdata;


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

    ctxt->indexnodes = indexnodes_new();

    method_trace_dedent();


    /* No reply */
}
