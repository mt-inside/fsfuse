/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Implementation of mkdir().
 */

#include "common.h"

#include <errno.h>

#include "fuse_methods.h"
#include "trace.h"


void fsfuse_mkdir (fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode)
{
    NOT_USED(parent);
    NOT_USED(name);
    NOT_USED(mode);

    method_trace("fsfuse_mkdir(parent %lu, name %s, mode %#x)\n", parent, name, mode);


    assert(!fuse_reply_err(req, EROFS));
}
