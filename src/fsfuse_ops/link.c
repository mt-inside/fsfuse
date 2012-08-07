/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Implementation of link().
 */

#include "common.h"

#include <fuse/fuse_lowlevel.h>
#include <errno.h>

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
