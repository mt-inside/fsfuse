/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Implementation of mknod().
 */

#include "common.h"

#include <errno.h>

#include "fuse_methods.h"
#include "trace.h"


void fsfuse_mknod (fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, dev_t rdev)
{
    NOT_USED(parent);
    NOT_USED(name);
    NOT_USED(mode);
    NOT_USED(rdev);

    method_trace("fsfuse_mknod(parent %lu, name %s, mode %#x, dev %#x)\n", parent, name, mode, rdev);


    assert(!fuse_reply_err(req, EROFS));
}
