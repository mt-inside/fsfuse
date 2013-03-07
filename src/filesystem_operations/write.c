/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Implementation of write().
 */

#include "common.h"

#include <fuse/fuse_lowlevel.h>
#include <errno.h>

#include "fsfuse_ops/fsfuse_ops.h"
#include "trace.h"


void fsfuse_write (fuse_req_t req, fuse_ino_t ino, const char *buf, size_t size, off_t off, struct fuse_file_info *fi)
{
    NOT_USED(ino);
    NOT_USED(buf);
    NOT_USED(size);
    NOT_USED(off);
    NOT_USED(fi);

    method_trace("fsfuse_write(ino %lu, buf %p, size %zu, off %lu)\n", ino, buf, size, off);


    assert(!fuse_reply_err(req, EROFS));
}
