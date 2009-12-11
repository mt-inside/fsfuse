/*
 * Implementation of readlink().
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


int fsfuse_readlink ( const char *path,
                      char * buf,
                      size_t len )
{
    NOT_USED(buf);
    NOT_USED(len);

    method_trace("fsfuse_readlink(path==%s). SHOULD NOT HAPPEN\n", path);

    /* We do not currently claim that there are any symlinks in an fsfuse
     * filesystem (although I envisage search and/or multiple file alternatives
     * will be implemented like this in future), so we shouldn't currently be
     * getting any queries about them */
    assert(0);


    return 0; /* success */
}
