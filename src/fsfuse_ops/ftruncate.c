/*
 * Implementation of ftruncate().
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


int fsfuse_ftruncate ( const char *path,
                       off_t offset,
                       struct fuse_file_info *fi )
{
    NOT_USED(path);
    NOT_USED(offset);
    NOT_USED(fi);

    method_trace("fsfuse_ftruncate(path==%s, offset=%ju)\n", path, offset);


    return -EROFS;
}
