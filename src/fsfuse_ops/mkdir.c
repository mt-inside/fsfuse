/*
 * Implementation of mkdir().
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


void fsfuse_mkdir (fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode)
{
    NOT_USED(parent);
    NOT_USED(name);
    NOT_USED(mode);

    method_trace("fsfuse_mkdir(parent %lu, name %s, mode %#x)\n", parent, name, mode);


    assert(!fuse_reply_err(req, EROFS));
}
