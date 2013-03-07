/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Implementation of fsync().
 */

#include "common.h"

#include <fuse/fuse_lowlevel.h>
#include <errno.h>

#include "fsfuse_ops/fsfuse_ops.h"
#include "trace.h"


void fsfuse_fsync (fuse_req_t req, fuse_ino_t ino, int datasync, struct fuse_file_info *fi)
{
    NOT_USED(ino);
    NOT_USED(datasync);
    NOT_USED(fi);

    method_trace("fsfuse_fsync(ino %ld, datasync %d)\n", ino, datasync);


    assert(!fuse_reply_err(req, EROFS));
}
