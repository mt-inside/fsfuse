/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Implementation of setattr().
 */

#include "common.h"

#include <errno.h>

#include "fuse_methods.h"
#include "trace.h"


void fsfuse_setattr (fuse_req_t req,
                     fuse_ino_t ino,
                     struct stat *attr,
                     int to_set,
                     struct fuse_file_info *fi)
{
    NOT_USED(ino);
    NOT_USED(attr);
    NOT_USED(to_set);
    NOT_USED(fi);

    method_trace("fsfuse_setattr(ino %lu, attr %p, to_set %d, fi %p)\n",
         ino, attr, to_set, fi);


    assert(!fuse_reply_err(req, EROFS));
}
