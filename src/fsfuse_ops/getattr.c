/*
 * Implementations for misc. filesystem operations.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#include <fuse/fuse_lowlevel.h>
#include <errno.h>
#include <string.h>

#include "common.h"
#include "fsfuse_ops/fsfuse_ops.h"
#include "trace.h"
#include "direntry.h"


void fsfuse_getattr (fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    int rc;
    direntry_t *de;
    struct stat stats;


    NOT_USED(fi);

    method_trace("fsfuse_getattr(ino %ld)\n", ino);
    method_trace_indent();

    rc = direntry_get_by_inode(ino, &de);

    if (!rc)
    {
        assert(de);

        memset(&stats, 0, sizeof(stats));
        direntry_de2stat(de, &stats);
        direntry_delete(CALLER_INFO de);
    }
    else
    {
        assert(!de);
    }

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
