/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Implementation of flush().
 */

#include "common.h"

#include <errno.h>

#include "fuse_methods.h"
#include "trace.h"


/* Called once per close() of a file descriptor. There may be more than one
 * close() per fd open()ed, as fds are duplicated by dup(), fork(), etc */
/* TODO: what implications does this have for lifetimes of stuff? Does anything
 * need _delete()ing? */
void fsfuse_flush (fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    NOT_USED(ino);
    NOT_USED(fi);

    method_trace("fsfuse_flush(ino %ld)\n", ino);


    assert(!fuse_reply_err(req, 0));
}
