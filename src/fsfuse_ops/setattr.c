/*
 * Implementation of setattr().
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


void fsfuse_setattr (fuse_req_t req,
                     fuse_ino_t ino,
                     struct stat *attr,
                     int to_set,
                     struct fuse_file_info *fi)
{
    NOT_USED(ino);
    NOT_USED(attr);
    NOT_USED(to_set);
    NOT_USED(fi);

    method_trace("fsfuse_setattr(ino %lu, attr %p, to_set %d, fi %p)\n",
         ino, attr, to_set, fi);


    assert(!fuse_reply_err(req, EROFS));
}
