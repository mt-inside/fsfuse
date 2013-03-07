/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Implementation of rename().
 */

#include "common.h"

#include <errno.h>

#include "fuse_methods.h"
#include "trace.h"


void fsfuse_rename (fuse_req_t req, fuse_ino_t parent, const char *name, fuse_ino_t newparent, const char *newname)
{
    NOT_USED(parent);
    NOT_USED(name);
    NOT_USED(newparent);
    NOT_USED(newname);

    method_trace(
        "fsfuse_rename(parent %lu, name %s, newparent %lu, newname %s)\n",
        parent, name, newparent, newname
    );


    assert(!fuse_reply_err(req, EROFS));
}
