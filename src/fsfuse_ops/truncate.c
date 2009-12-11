/*
 * Implementation of truncate().
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


int fsfuse_truncate ( const char *path,
                      off_t offset      )
{
    method_trace("fsfuse_truncate(path==%s, offset=%ju)\n", path, offset);


    return -EROFS;
}
