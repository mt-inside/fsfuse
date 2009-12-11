/*
 * Implementation of mkdir().
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


int fsfuse_mkdir ( const char *path,
                   mode_t mode       )
{
    method_trace("fsfuse_mkdir(path==%s, mode=%#x)\n", path, mode);


    return -EROFS;
}
