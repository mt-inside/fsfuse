/*
 * Implementation of rename().
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


void fsfuse_rename (fuse_req_t req, fuse_ino_t parent, const char *name, fuse_ino_t newparent, const char *newname)
{
    NOT_USED(parent);
    NOT_USED(name);
    NOT_USED(newparent);
    NOT_USED(newname);

    method_trace(
        "fsfuse_rename(parent %lu, name %s, newparent %lu, newname %s)\n",
        parent, name, newparent, newname
    );


    assert(!fuse_reply_err(req, EROFS));
}
