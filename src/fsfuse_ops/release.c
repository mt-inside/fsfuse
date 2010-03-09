/*
 * Implementation of release().
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


/* Called exactly once per open fd - when the last instance of that fd is
 * close()ed / munmap()ped. Thus there will be one release() per open().
 */
void fsfuse_release (fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    NOT_USED(ino);
    NOT_USED(fi);

    method_trace("fsfuse_release(ino %lu)\n", ino);


    /* TODO */

    /* Any error codes passed in here are not passed on to userspace. */
    assert(!fuse_reply_err(req, 0));
}
