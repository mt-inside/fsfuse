/*
 * readdir() declaration.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#ifndef _INCLUDED_READDIR_H
#define _INCLUDED_READDIR_H

extern int fsfuse_readdir (const char *path,
                           void *buf,
                           fuse_fill_dir_t filler,
                           off_t offset,
                           struct fuse_file_info *fi);

#endif /* _INCLUDED_READDIR_H */
