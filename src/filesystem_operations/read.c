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
#include <fuse/fuse_lowlevel.h>
#include <stdlib.h>

#include "locks.h"
#include "fsfuse_ops/fsfuse_ops.h"
#include "direntry.h"
#include "download_thread_pool.h"


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
 */
void fsfuse_read (fuse_req_t req,
                  fuse_ino_t ino,
                  size_t size,
                  off_t off,
                  struct fuse_file_info *fi)
{
    int rc; /* +ve: number of bytes read
               -ve: -ERRNO */
    direntry_t *de;


    NOT_USED(fi);

    method_trace("fsfuse_read(ino %lu, size %zd, off %ju)\n", ino, size, off);
    method_trace_indent();


    rc = direntry_get_by_inode(ino, &de);

    if (!rc)
    {
        read_context_t *read_ctxt = (read_context_t *)calloc(sizeof(read_context_t), 1);
        void *buf;


        /* Userspace, and therefore fuse, can ask for any range of bytes,
         * regardless of the file length. E.g. ext2 just gets over it. If:
         *   start > length: return 0 bytes
         *   end   > length: return length - start bytes.
         */
        if (off >= direntry_get_size(de))
        {
            size = 0;
        }
        else if ((unsigned)(off + size) >= direntry_get_size(de))
        {
            size = direntry_get_size(de) - off;
        }

        buf = malloc(size);

        read_ctxt->req  = req;
        read_ctxt->de   = de;
        read_ctxt->size = size;
        read_ctxt->buf  = buf;

        download_thread_pool_chunk_add(de, off, off + size, buf, &chunk_done, (void *)read_ctxt);
    }
    else
    {
        fuse_reply_err(req, rc);
    }

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
