/*
 * Implementation of releasedir().
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


/* Called exactly once per opendir().
 * This does not mean that there won't be more operations on that directory
 * without a subsequent opendir() - there could be other open fds also
 * referring to the directory in question.
 */
int fsfuse_releasedir ( const char * path,
                        struct fuse_file_info *fi )
{
    method_trace("fsfuse_releasedir(path==%s)\n", path);

    NOT_USED(fi);


    return 0;
}
