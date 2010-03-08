/*
 * Implementation of write().
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#include <fuse/fuse_lowlevel.h>
#include <errno.h>

#include "common.h"
#include "fsfuse_ops/fsfuse_ops.h"
#include "trace.h"


void fsfuse_write (fuse_req_t req, fuse_ino_t ino, const char *buf, size_t size, off_t off, struct fuse_file_info *fi)
{
    NOT_USED(ino);
    NOT_USED(buf);
    NOT_USED(size);
    NOT_USED(off);
    NOT_USED(fi);

    method_trace("fsfuse_write(ino %lu, buf %p, size %zu, off %lu)\n", ino, buf, size, off);


    assert(!fuse_reply_err(req, EROFS));
}
