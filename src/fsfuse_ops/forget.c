/*
 * Implementation of forget().
 *
 * Copyright (C) Matthew Turner 2008-2010. All rights reserved.
 *
 * $Id: bmap.c 441 2010-01-11 23:56:17Z matt $
 */

#include "common.h"

#include <fuse/fuse_lowlevel.h>
#include <errno.h>

#include "fsfuse_ops/fsfuse_ops.h"
#include "trace.h"


void fsfuse_forget (fuse_req_t req,
                    fuse_ino_t ino,
                    unsigned long nlookup)
{
    NOT_USED(ino);
    NOT_USED(nlookup);

    method_trace("fsfuse_forget(ino %lu, nlookup %lu)\n",
         ino, nlookup);

    /* TODO */

    fuse_reply_none(req);
}
