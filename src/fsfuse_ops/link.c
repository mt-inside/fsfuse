/*
 * Implementation of link().
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


void fsfuse_link (fuse_req_t req, fuse_ino_t ino, fuse_ino_t newparent, const char *newname)
{
    NOT_USED(ino);
    NOT_USED(newparent);
    NOT_USED(newname);

    method_trace("fsfuse_link(ino %ld, newparent %ld, newname %s)\n", ino, newparent, newname);


    assert(!fuse_reply_err(req, EROFS));
}
