/*
 * Implementation of bmap().
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#include "common.h"

#include <fuse/fuse_lowlevel.h>
#include <errno.h>

#include "fsfuse_ops/fsfuse_ops.h"
#include "trace.h"


/* Totally pointless operation on a pretend filesystem not stored on a block
 * device, which is read-only anyway. I'd like to see anyone swap onto fsfuse,
 * or install a bootloader on it */
void fsfuse_bmap (fuse_req_t req, fuse_ino_t ino, size_t blocksize, uint64_t idx)
{
    NOT_USED(ino);
    NOT_USED(blocksize);
    NOT_USED(idx);

    method_trace("fsfuse_bmap(ino %ld, blocksize %zu, idx %lu)\n",
         ino, blocksize, idx);

    trace_warn("bmap would seem rather pointless... Anything calling it is likely to malfunction pretty quickly\n");


    assert(!fuse_reply_err(req, EIO));
}
