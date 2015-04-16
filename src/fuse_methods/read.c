/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * read() implementation.
 */

#include "common.h"

#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "locks.h"
#include "fuse_methods.h"
#include "direntry.h"
#include "downloader.h"


TRACE_DEFINE(read)


typedef struct
{
    direntry_t *de;
    fuse_req_t req;
    size_t size; /* requested size */
    void *buf;
} read_context_t;


static void chunk_done (void *read_ctxt, int rc, size_t size);


/* MUST return as many bytes as were asked for, unless error or EOF.
 *
 * Interesting snippets from the fuse docs:
 *   max_read=N
 *     With this option the maximum size of read operations can be set.
 *     The default is infinite.  Note that the size of read requests is
 *     limited anyway to 32 pages (which is 128kbyte on i386).
 *
 *   max_readahead=N
 *     Set the maximum number of bytes to read-ahead.  The default is
 *     determined by the kernel.  On linux-2.6.22 or earlier it's 131072
 *     (128kbytes)
 *   - maybe this explains the over-reads that we get.
 *
 * The FUSE docs also assert that read() will only be called on a file that's
 * been successfully open()ed. This means we don't have to check it's a file,
 * its permissions, etc. We /do/ have to check it (still) exists though.
 *
 * Note that we still do the network round-trip for 0 byte ranges. This is
 * because userspace could have made that request for whatever reason (e.g. to
 * measure latency) and they have morally "interacted" with the file, so we
 * should see if it still exists and return error if it doesn't.
 *
 * TODO: this can just get de from the fi, as open() will have filled it in. No
 * need to check that's it's a file or permissions or anything, but it (or its
 * whole indexnode) might not exist any more.
 * Open files should own download threads (should be spun up* in open()) that
 * get stashed in fi. fi->thread then gets told (not asked) to get a chunk. The
 * continuation is still chunk_done(), which can either return enoent, or a
 * buffer
 */
void fsfuse_read (fuse_req_t req,
                  fuse_ino_t ino,
                  size_t size,
                  off_t off,
                  struct fuse_file_info *fi)
{
    open_file_ctxt_t *ctxt = (open_file_ctxt_t *)fi->fh;
    read_context_t *read_ctxt = (read_context_t *)calloc(sizeof(read_context_t), 1);
    void *buf;


    NOT_USED(ino);

    method_trace("fsfuse_read(ino %lu, size %zd, off %ju)\n", ino, size, off);
    method_trace_indent();


    /* Userspace, and therefore fuse, can ask for any range of bytes,
     * regardless of the file length. E.g. ext2 just gets over it. If:
     *   start > length: return 0 bytes
     *   end   > length: return length - start bytes.
     */
    if (off >= direntry_get_size(ctxt->de))
    {
        size = 0;
    }
    else if ((unsigned)(off + size) >= direntry_get_size(ctxt->de))
    {
        size = direntry_get_size(ctxt->de) - off;
    }

    buf = malloc(size);

    read_ctxt->req  = req;
    read_ctxt->de   = ctxt->de;
    read_ctxt->size = size;
    read_ctxt->buf  = buf;

    downloader_chunk_add(ctxt->downloader, off, off + size, buf, &chunk_done, (void *)read_ctxt);

    method_trace_dedent();
}

/* This, and hence fuse_reply, are called on a different thread to the one the
 * request came in on. This doesn't seem to matter. */
static void chunk_done (void *read_ctxt, int rc, size_t size)
{
    read_context_t *ctxt = (read_context_t *)read_ctxt;


    if (size != ctxt->size)
    {
        read_trace("bytes read != request size => EOF / error\n");
    }

    /* As a result of this read() operation we can say certain things
     * about the direntry. These professions can be made with confidence
     * because we've just performed an actual network transaction, so
     * our information is "live":
     * - If we actually read data from the file then it definitely still
     *   exists, now.
     * - If we tried to read from it and found it didn't exist, then it
     *   definitely doesn't exist any more.
     * - Any other error doesn't give enough info.
     */
    if (!rc)
    {
        direntry_still_exists(ctxt->de);
    }
    else if (rc == ENOENT)
    {
        direntry_no_longer_exists(ctxt->de);
    }


    if (!rc)
    {
        assert(!fuse_reply_buf(ctxt->req, ctxt->buf, size));
    }
    else
    {
        assert(!fuse_reply_err(ctxt->req, rc));
    }

    direntry_delete(CALLER_INFO ctxt->de);
    free(ctxt->buf);
    free(read_ctxt);
}
