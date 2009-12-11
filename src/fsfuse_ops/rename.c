/*
 * Implementation of rename().
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


int fsfuse_rename ( const char *from,
                    const char *to    )
{
    method_trace("fsfuse_rename(from==%s, to==%s)\n", from, to);


    return -EROFS;
}
