/*
 * Implementation of mknod().
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


int fsfuse_mknod ( const char *path,
                   mode_t mode,
                   dev_t dev         )
{
    NOT_USED(path);
    NOT_USED(mode);
    NOT_USED(dev);

    method_trace("fsfuse_mknod(path==%s, mode=%#x, dev=%#x)\n", path, mode, dev);


    return -EROFS;
}
