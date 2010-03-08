/*
 * Implementation of rmdir().
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


void fsfuse_rmdir (fuse_req_t req, fuse_ino_t parent, const char *name)
{
    NOT_USED(parent);
    NOT_USED(name);

    method_trace("fsfuse_rmdir(parent %lu, name %s)\n", parent, name);


    assert(!fuse_reply_err(req, EROFS));
}
