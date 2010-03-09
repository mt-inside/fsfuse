/*
 * Implementation of fsync().
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


void fsfuse_fsync (fuse_req_t req, fuse_ino_t ino, int datasync, struct fuse_file_info *fi)
{
    NOT_USED(ino);
    NOT_USED(datasync);
    NOT_USED(fi);

    method_trace("fsfuse_fsync(ino %ld, datasync %d)\n", ino, datasync);


    assert(!fuse_reply_err(req, EROFS));
}
