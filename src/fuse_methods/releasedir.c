/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Implementation of releasedir().
 */

#include "common.h"

#include <errno.h>

#include "direntry.h"
#include "fuse_methods.h"
#include "trace.h"


/* Called exactly once per opendir().
 * This does not mean that there won't be more operations on that directory
 * without a subsequent opendir() - there could be other open fds also
 * referring to the directory in question.
 */
void fsfuse_releasedir (fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    int rc;
    direntry_t *de;


    NOT_USED(fi);

    method_trace("fsfuse_releasedir(ino %lu)\n", ino);
    method_trace_indent();

    rc = direntry_get_by_inode(ino, &de);

    if (!rc)
    {
        /* Delete our copy, and the one taken by opendir() */
        direntry_delete(CALLER_INFO de);
        direntry_delete(CALLER_INFO de);
    }

    method_trace_dedent();


    /* Any error codes passed in here are not passed on to userspace. */
    assert(!fuse_reply_err(req, rc));
}
