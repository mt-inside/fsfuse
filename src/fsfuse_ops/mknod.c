/*
 * Implementation of mknod().
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


void fsfuse_mknod (fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, dev_t rdev)
{
    NOT_USED(parent);
    NOT_USED(name);
    NOT_USED(mode);
    NOT_USED(rdev);

    method_trace("fsfuse_mknod(parent %lu, name %s, mode %#x, dev %#x)\n", parent, name, mode, rdev);


    assert(!fuse_reply_err(req, EROFS));
}
