/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Implementation of release().
 */

#include "common.h"

#include <errno.h>
#include <stdlib.h>

#include "direntry.h"
#include "fuse_methods.h"
#include "trace.h"


/* Called exactly once per open fd - when the last instance of that fd is
 * close()ed / munmap()ped. Thus there will be one release() per open().
 */
/* TODO: what implications does this have for lifetimes of stuff? Does anything
 * need _delete()ing?
 * TODO: When we move to direntries (thus return the same inode/object every
 * time), they should probably have an open() count, at least for
 * interest/assertions
 * - need to make the decision whether de's own download threads (in which case
 *   the open() count will ref-count them) or whether open()s own download
 *   threads. */
void fsfuse_release (fuse_req_t req,
                     fuse_ino_t ino,
                     struct fuse_file_info *fi)
{
    open_file_ctxt_t *ctxt = (open_file_ctxt_t *)fi->fh;


    NOT_USED(ino);

    method_trace("fsfuse_release(ino %lu)\n", ino);
    method_trace_indent();

    /* Delete our copy, and the one taken by open() */
    direntry_delete(CALLER_INFO ctxt->de);
    direntry_delete(CALLER_INFO ctxt->de);

    //downloader_delete( ctxt->downloader ); TODO when this is public

    free(ctxt);

    method_trace_dedent();


    /* Any error codes passed in here are not passed on to userspace anyhow.
     * flush() should return errors to be returned on close(). */
    assert(!fuse_reply_err(req, 0));
}
