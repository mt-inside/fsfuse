/*
 * Implementations for misc. filesystem operations.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#include <fuse/fuse_lowlevel.h>
#include <errno.h>

#include "common.h"
#include "fsfuse_ops/fsfuse_ops.h"
#include "trace.h"
#include "direntry.h"


void fsfuse_open (fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    int rc;
    direntry_t *de;


    method_trace("fsfuse_open(ino %lu)\n", ino);

    rc = direntry_get_by_inode(ino, &de);

    if (!rc)
    {
        /* Ordering below is deliberate - the reverse of our order of precedence
         * for complaining (TODO: which is a guess anyway). */
        if ((fi->flags & 3) != O_RDONLY)                 rc = EROFS;
        if (direntry_get_type(de) != direntry_type_FILE) rc = EISDIR;

        direntry_delete(CALLER_INFO de);
    }


    if (!rc)
    {
        assert(!fuse_reply_open(req, fi));
    }
    else
    {
        assert(!fuse_reply_err(req, rc));
    }
}

