/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Implementation of readlink().
 */

#include "common.h"

#include <errno.h>

#include "fuse_methods.h"
#include "trace.h"


void fsfuse_readlink (fuse_req_t req, fuse_ino_t ino)
{
    NOT_USED(ino);

    method_trace("fsfuse_readlink(ino %lu)\n", ino);

    /* We do not currently claim that there are any symlinks in an fsfuse
     * filesystem (although I envisage search and/or multiple file alternatives
     * will be implemented like this in future), so we shouldn't currently be
     * getting any queries about them */
    assert(0);


    assert(!fuse_reply_err(req, EIO));
}
