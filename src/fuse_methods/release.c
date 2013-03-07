/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Implementation of release().
 */

#include "common.h"

#include <errno.h>

#include "direntry.h"
#include "fuse_methods.h"
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
