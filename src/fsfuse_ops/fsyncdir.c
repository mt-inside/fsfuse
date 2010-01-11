/*
 * Implementation of fsyncdir().
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


int fsfuse_fsyncdir ( const char *path,
                      int datasync,
                      struct fuse_file_info *fi )
{
    NOT_USED(path);
    NOT_USED(datasync);
    NOT_USED(fi);

    method_trace("fsfuse_fsyncdir(path==%s, datasync==%d)\n", path, datasync);


    return -EROFS;
}
