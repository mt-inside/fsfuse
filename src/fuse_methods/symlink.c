/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Implementation of symlink().
 */

#include "common.h"

#include <errno.h>

#include "fuse_methods.h"
#include "trace.h"


void fsfuse_symlink (fuse_req_t req, const char *link, fuse_ino_t parent, const char *name)
{
    NOT_USED(link);
    NOT_USED(parent);
    NOT_USED(name);

    method_trace("fsfuse_symlink(link %s, parent %lu, name %s)\n", link, parent, name);


    assert(!fuse_reply_err(req, EROFS));
}
