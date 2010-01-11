/*
 * Implementation of flush().
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


/* Called once per close() of a file descriptor. There may be more than one
 * close() per fd open()ed, as fds are duplicated by dup(), fork(), etc */
int fsfuse_flush ( const char *path,
                   struct fuse_file_info *fi )
{
    NOT_USED(path);
    NOT_USED(fi);

    method_trace("fsfuse_flush(path==%s)\n", path);


    return 0;
}
