/*
 * Implementation of bmap().
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


/* Totally pointless operation on a pretend filesystem not stored on a block
 * device, which is read-only anyway. I'd like to see anyone swap onto fsfuse,
 * or install a bootloader on it */
int fsfuse_bmap (const char *path,
                 size_t blocksize,
                 uint64_t *idx)
{
    method_trace("fsfuse_bmap(path==%s, blocksize=%zu, idx==%lu)\n",
         path, blocksize, idx);
    trce("bmap would seem rather pointless... Anything calling it is likely to malfunction pretty quickly\n");
    assert(0); /* bmap? Presumably, someone could call this and then try to
                  write to some "underlying" block device, not that we report
                  one, at which point things will go horribly wrong. */


    return 0;
}
