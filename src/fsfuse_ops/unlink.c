/*
 * Implementation of unlink().
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


int fsfuse_unlink ( const char *path )
{
    NOT_USED(path);

    method_trace("fsfuse_unlink(path==%s)\n", path);


    return -EROFS;
}
