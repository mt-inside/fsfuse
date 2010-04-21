/*
 * Implementation of forget().
 *
 * Copyright (C) Matthew Turner 2008-2010. All rights reserved.
 *
 * $Id: bmap.c 441 2010-01-11 23:56:17Z matt $
 */

#include "common.h"

#include <fuse/fuse_lowlevel.h>
#include <errno.h>

#include "direntry.h"
#include "fsfuse_ops/fsfuse_ops.h"
#include "trace.h"


void fsfuse_forget (fuse_req_t req,
                    fuse_ino_t ino,
                    unsigned long nlookup)
{
    int rc;
    direntry_t *de;


    method_trace("fsfuse_forget(ino %lu, nlookup %lu)\n",
         ino, nlookup);
    method_trace_indent();

    rc = direntry_get_by_inode(ino, &de);

    if (!rc)
    {
        /* Delete our copy... */
        direntry_delete(CALLER_INFO de);

        /* ...and all the ones taken by lookup() */
        while (nlookup--)
        {
            direntry_delete(CALLER_INFO de);
        }
    }

    method_trace_dedent();


    fuse_reply_none(req);
}
