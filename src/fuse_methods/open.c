/*
 * Copyright (C) 2008-2013 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * open()
 */

#include "common.h"

#include <errno.h>
#include <stdlib.h>

#include "fuse_methods.h"
#include "trace.h"
#include "direntry.h"


void fsfuse_open (fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    int rc;
    direntry_t *de;


    method_trace("fsfuse_open(ino %lu)\n", ino);
    method_trace_indent();

    rc = direntry_get_by_inode(ino, &de);

    if (!rc)
    {
        /* Ordering below is deliberate - the reverse of our order of precedence
         * for complaining (TODO: which is a guess anyway). */
        if ((fi->flags & 3) != O_RDONLY)                rc = EROFS;
        if (direntry_get_type(de) != listing_type_FILE) rc = EISDIR;

        if (!rc)
        {
            open_file_ctxt_t *ctxt = malloc( sizeof(*ctxt) );
            ctxt->de = de;
            ctxt->downloader = downloader_new( de );
            fi->fh = (typeof(fi->fh))ctxt;
        }
    }

    method_trace_dedent();


    /* Don't delete the de, as the success branch usurps the copy to be owned by
     * the open file, and the error branch doesn't have a de to delete */
    if (!rc)
    {
        assert(!fuse_reply_open(req, fi));
    }
    else
    {
        assert(!fuse_reply_err(req, rc));
    }
}
