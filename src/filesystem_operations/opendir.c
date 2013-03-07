/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Implementations for misc. filesystem operations.
 */

#include "common.h"

#include <fuse/fuse_lowlevel.h>
#include <errno.h>

#include "fsfuse_ops/fsfuse_ops.h"
#include "trace.h"
#include "direntry.h"


void fsfuse_opendir (fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    /* For now, simply check the existence of path, and check permissions */
    int rc;
    direntry_t *de;


    method_trace("fsfuse_opendir(ino %lu)\n", ino);
    method_trace_indent();

    rc = direntry_get_by_inode(ino, &de);

    if (!rc)
    {
        /* can you open a directory for write? The libc function doesn't take
         * any flags... */
        /* Ordering below is deliberate - the reverse of our order of presidence
         * for complaining (TODO: which is a guess anyway). */
        if ((fi->flags & 3) != O_RDONLY)                     rc = EROFS;
        if (direntry_get_type(de) != listing_type_DIRECTORY) rc = ENOTDIR;
    }

    method_trace_dedent();


    /* Don't delete the de, as the success branch usurps the copy to be owned by
     * the open directory, and the error branch doesn't have a de to delete */
    if (!rc)
    {
        assert(!fuse_reply_open(req, fi));
    }
    else
    {
        assert(!fuse_reply_err(req, rc));
    }
}
