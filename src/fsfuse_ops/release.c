/*
 * Implementation of release().
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#include "common.h"

#include <fuse/fuse_lowlevel.h>
#include <errno.h>

#include "direntry.h"
#include "fsfuse_ops/fsfuse_ops.h"
#include "trace.h"


/* Called exactly once per open fd - when the last instance of that fd is
 * close()ed / munmap()ped. Thus there will be one release() per open().
 */
void fsfuse_release (fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    int rc;
    direntry_t *de;


    NOT_USED(fi);

    method_trace("fsfuse_release(ino %lu)\n", ino);
    method_trace_indent();

    rc = direntry_get_by_inode(ino, &de);

    if (!rc)
    {
        /* Delete our copy, and the one taken by open() */
        direntry_delete(CALLER_INFO de);
        direntry_delete(CALLER_INFO de);
    }

    method_trace_dedent();


    /* Any error codes passed in here are not passed on to userspace. */
    assert(!fuse_reply_err(req, rc));
}
