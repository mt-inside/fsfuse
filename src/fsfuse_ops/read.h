/*
 * read() declaration.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#ifndef _INCLUDED_READ_H
#define _INCLUDED_READ_H

#include <fuse.h>


TRACE_DECLARE(read)


extern int fsfuse_read (const char *path,
                        char *buf,
                        size_t size,
                        off_t offset,
                        struct fuse_file_info *fi);

extern void read_signal_chunk_done (int rc, void *ctxt);

#endif /* _INCLUDED_READ_H */
