/*
 * Declarations for misc. filesystem operations.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#include <fuse.h>

#include "common.h"
#include "trace.h"


TRACE_DECLARE(method)


extern int fsfuse_getattr ( const char *path,
                            struct stat *stbuf );

extern int fsfuse_readlink ( const char *path,
                             char * buf,
                             size_t len );

extern int fsfuse_open ( const char *path,
                         struct fuse_file_info *fi );

extern int fsfuse_mknod ( const char *path,
                          mode_t mode,
                          dev_t dev         );

extern int fsfuse_mkdir ( const char *path,
                          mode_t mode       );

extern int fsfuse_unlink ( const char *path );

extern int fsfuse_rmdir ( const char *path );

extern int fsfuse_symlink ( const char *from,
                            const char *to    );

extern int fsfuse_rename ( const char *from,
                           const char *to    );

extern int fsfuse_link ( const char *from,
                         const char *to    );

extern int fsfuse_chmod ( const char *path,
                          mode_t mode       );

extern int fsfuse_chown ( const char *path,
                          uid_t user,
                          gid_t group       );

extern int fsfuse_truncate ( const char *path,
                             off_t offset      );

extern int fsfuse_write ( const char *path,
                          const char *buf,
                          size_t size,
                          off_t off,
                          struct fuse_file_info *fi );

extern int fsfuse_flush ( const char *path,
                   struct fuse_file_info *fi );

extern int fsfuse_release ( const char *path,
                     struct fuse_file_info *fi );

extern int fsfuse_fsync ( const char *path,
                          int datasync,
                          struct fuse_file_info *fi );

extern int fsfuse_opendir ( const char *path,
                            struct fuse_file_info *fi );

extern int fsfuse_releasedir ( const char * path,
                               struct fuse_file_info *fi );

extern int fsfuse_fsyncdir ( const char *path,
                             int datasync,
                             struct fuse_file_info *fi );

extern int fsfuse_create ( const char *path,
                           mode_t mode,
                           struct fuse_file_info *fi );

extern int fsfuse_ftruncate ( const char *path,
                              off_t offset,
                              struct fuse_file_info *fi );

extern int fsfuse_access ( const char *path,
                           int mode          );

extern int fsfuse_bmap (const char *,
                        size_t blocksize,
                        uint64_t *idx);
