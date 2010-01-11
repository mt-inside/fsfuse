/*
 * Implementation of chmod().
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


int fsfuse_chmod ( const char *path,
                   mode_t mode       )
{
    NOT_USED(path);
    NOT_USED(mode);

    method_trace("fsfuse_chmod(path==%s, mode=%#x)\n", path, mode);


    return -EROFS;
}
