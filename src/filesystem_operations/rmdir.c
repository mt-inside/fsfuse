/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Implementation of rmdir().
 */

#include "common.h"

#include <fuse/fuse_lowlevel.h>
#include <errno.h>

#include "fsfuse_ops/fsfuse_ops.h"
#include "trace.h"


void fsfuse_rmdir (fuse_req_t req, fuse_ino_t parent, const char *name)
{
    NOT_USED(parent);
    NOT_USED(name);

    method_trace("fsfuse_rmdir(parent %lu, name %s)\n", parent, name);


    assert(!fuse_reply_err(req, EROFS));
}
