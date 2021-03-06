/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Implementation of fsyncdir().
 */

#include "common.h"

#include <errno.h>

#include "fuse_methods.h"
#include "trace.h"


void fsfuse_fsyncdir (fuse_req_t req, fuse_ino_t ino, int datasync, struct fuse_file_info *fi)
{
    NOT_USED(ino);
    NOT_USED(datasync);
    NOT_USED(fi);

    method_trace("fsfuse_fsyncdir(ino %s, datasync d)\n", ino, datasync);


    assert(!fuse_reply_err(req, EROFS));
}
