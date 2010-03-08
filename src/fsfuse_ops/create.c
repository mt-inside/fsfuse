/*
 * Implementation of create().
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


void fsfuse_create (fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, struct fuse_file_info *fi)
{
    NOT_USED(parent);
    NOT_USED(name);
    NOT_USED(mode);
    NOT_USED(fi);

    method_trace("fsfuse_create(parent %ld, name %s, mode %#x)\n", parent, name, mode);


    assert(!fuse_reply_err(req, EROFS));
}
