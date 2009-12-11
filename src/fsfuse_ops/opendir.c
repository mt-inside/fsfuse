/*
 * Implementations for misc. filesystem operations.
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
#include "direntry.h"


int fsfuse_opendir ( const char *path,
                     struct fuse_file_info *fi )
{
    /* For now, simply check the existence of path, and check permissions */
    int rc;
    direntry_t *de;


    method_trace("fsfuse_opendir(path==%s)\n", path);

    rc = path_get_direntry(path, &de);

    if (!rc)
    {
        /* can you open a directory for write? The libc function doesn't take
         * any flags... */
        /* Ordering below is deliberate - the reverse of our order of presidence
         * for complaining (TODO: which is a guess anyway). */
        if ((fi->flags & 3) != O_RDONLY)                      rc = -EROFS;
        if (direntry_get_type(de) != direntry_type_DIRECTORY) rc = -ENOTDIR;

        direntry_delete(CALLER_INFO de);
    }


    return rc;
}

