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


int fsfuse_getattr ( const char *path,
                     struct stat *stbuf )
{
    int rc;
    direntry_t *de;


    method_trace("fsfuse_getattr(%s)\n", path);
    method_trace_indent();

    rc = path_get_direntry(path, &de);

    if (!rc)
    {
        assert(de);

        direntry_de2stat(stbuf, de);
        direntry_delete(CALLER_INFO de);
    }
    else
    {
        assert(!de);
    }

    method_trace_dedent();


    return rc;
}

