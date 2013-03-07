/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Implementation of forget().
 */

#include "common.h"

#include <errno.h>

#include "direntry.h"
#include "fuse_methods.h"
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
