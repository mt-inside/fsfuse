/*
 * Implementation of access().
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#include "common.h"

#include <fuse/fuse_lowlevel.h>
#include <errno.h>

#include "trace.h"
#include "config.h"
#include "fsfuse_ops/fsfuse_ops.h"
#include "direntry.h"


void fsfuse_access (fuse_req_t req, fuse_ino_t ino, int mask)
{
    int rc = 0;
    direntry_t *de;


    method_trace("fsfuse_access(ino %ld, mask %o)\n", ino, mask);
    method_trace_indent();

    rc = direntry_get_by_inode(ino, &de);

    if (!rc)
    {
        switch (direntry_get_type(de))
        {
            case direntry_type_DIRECTORY:
                if (mask & ~config_attr_mode_dir)  rc = EACCES;
                break;
            case direntry_type_FILE:
                if (mask & ~config_attr_mode_file) rc = EACCES;
                break;
        }

        direntry_delete(CALLER_INFO de);
    }

    method_trace_dedent();


    assert(!fuse_reply_err(req, rc));
}

