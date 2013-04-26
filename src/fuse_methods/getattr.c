/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Implementations for misc. filesystem operations.
 */

#include "common.h"

#include <errno.h>
#include <string.h>

#include "fuse_methods.h"
#include "trace.h"
#include "direntry.h"


void fsfuse_getattr (fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    int rc;
    direntry_t *de = (direntry_t *)fi->fh;
    struct stat stats;


    method_trace("fsfuse_getattr(ino %ld)\n", ino);
    method_trace_indent();

    memset(&stats, 0, sizeof(stats));
    direntry_de2stat(de, &stats);
    direntry_delete(CALLER_INFO de);

    method_trace_dedent();


    if (rc)
    {
        assert(!fuse_reply_err(req, rc));
    }
    else
    {
        /* TODO: timeout value below. The fuse example takes its struct stat off
         * the stack and passes it into reply_attr, so it must get copied during
         * the call.
         * The timeout, I assume, therefore indicates how long to trust those
         * stats for before requesting them again. */
        assert(!fuse_reply_attr(req, &stats, 1.0));
    }
}
