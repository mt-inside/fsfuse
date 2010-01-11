/*
 * Implementation of release().
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


/* Called exactly once per open fd - when the last instance of that fd is
 * close()ed. Thus there will be one release() per open().
 * This does not mean that there won't be more operations on that file without a
 * subsequent open() - there could be other open fds also referring to the file
 * in question.
 */
int fsfuse_release ( const char *path,
                     struct fuse_file_info *fi )
{
    NOT_USED(path);
    NOT_USED(fi);

    method_trace("fsfuse_release(path==%s)\n", path);


    return 0;
}
