/*
 * Implementation of readlink().
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


void fsfuse_readlink (fuse_req_t req, fuse_ino_t ino)
{
    NOT_USED(ino);

    method_trace("fsfuse_readlink(ino %lu)\n", ino);

    /* We do not currently claim that there are any symlinks in an fsfuse
     * filesystem (although I envisage search and/or multiple file alternatives
     * will be implemented like this in future), so we shouldn't currently be
     * getting any queries about them */
    assert(0);


    assert(!fuse_reply_err(req, EIO));
}
