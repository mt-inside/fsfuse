/*
 * Implementation of create().
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


int fsfuse_create ( const char *path,
                    mode_t mode,
                    struct fuse_file_info *fi )
{
    method_trace("fsfuse_create(path==%s, mode==%#x)\n", path, mode);

    NOT_USED(fi);


    return -EROFS;
}
