/*
 * Implementation of access().
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#include <fuse.h>
#include <errno.h>

#include "common.h"
#include "trace.h"
#include "config.h"
#include "fsfuse_ops/fsfuse_ops.h"
#include "direntry.h"


int fsfuse_access ( const char *path,
                    int mode          )
{
    int rc = 0;
    direntry_t *de;


    method_trace("fsfuse_access(%s, %o)\n", path, mode);
    method_trace_indent();

    rc = path_get_direntry(path, &de);

    if (!rc)
    {
        switch (direntry_get_type(de))
        {
            case direntry_type_DIRECTORY:
                if (mode & ~config_attr_mode_dir)  rc = -EACCES;
                break;
            case direntry_type_FILE:
                if (mode & ~config_attr_mode_file) rc = -EACCES;
                break;
        }

        direntry_delete(CALLER_INFO de);
    }

    method_trace_dedent();


    return rc;
}

