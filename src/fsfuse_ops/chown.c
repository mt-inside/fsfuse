/*
 * Implementation of chown().
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


int fsfuse_chown ( const char *path,
                   uid_t user,
                   gid_t group       )
{
    method_trace("fsfuse_chown(path==%s, user=%d, group=%d)\n", path, user, group);


    return -EROFS;
}
