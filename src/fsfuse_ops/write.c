/*
 * Implementation of write().
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#include <fuse.h>
#include <errno.h>

#include "common.h"
#include "fsfuse_ops/fsfuse_ops.h"
#include "trace.h"


int fsfuse_write ( const char *path,
                   const char *buf,
                   size_t size,
                   off_t off,
                   struct fuse_file_info *fi )
{
    method_trace("fsfuse_write(path==%s, size==%zd, off==%ju)\n", path, size, off);

    NOT_USED(buf);
    NOT_USED(off);
    NOT_USED(fi);


    return -EROFS;
}
