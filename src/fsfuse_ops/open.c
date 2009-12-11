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


int fsfuse_open ( const char *path,
                  struct fuse_file_info *fi )
{
    /* not doing anything special here atm (future posibilities include
     * pre-fetching (incl. parallel pre-fetching in another thread))
     * For now, simply check the existance of path, and check permissions */
    int rc;
    direntry_t *de;


    method_trace("fsfuse_open(path==%s)\n", path);

    rc = path_get_direntry(path, &de);

    if (!rc)
    {
        /* Ordering below is deliberate - the reverse of our order of presidence
         * for complaining (TODO: which is a guess anyway). */
        if ((fi->flags & 3) != O_RDONLY)                 rc = -EROFS;
        if (direntry_get_type(de) != direntry_type_FILE) rc = -EISDIR;

        direntry_delete(CALLER_INFO de);
    }


    return rc;
}

