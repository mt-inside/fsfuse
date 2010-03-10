/*
 * Implementation of releasedir().
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#include "common.h"

#include <fuse/fuse_lowlevel.h>
#include <errno.h>

#include "fsfuse_ops/fsfuse_ops.h"
#include "trace.h"


/* Called exactly once per opendir().
 * This does not mean that there won't be more operations on that directory
 * without a subsequent opendir() - there could be other open fds also
 * referring to the directory in question.
 */
void fsfuse_releasedir (fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    NOT_USED(ino);
    NOT_USED(fi);

    method_trace("fsfuse_releasedir(ino %lu)\n", ino);
    method_trace_indent();


    /* TODO */

    method_trace_dedent();


    /* Any error codes passed in here are not passed on to userspace. */
    assert(!fuse_reply_err(req, 0));
}
