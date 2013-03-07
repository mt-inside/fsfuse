/*
 * Copyright (C) 2008-2013 Matthew Turner.
 *
 * Declarations of all fuse methods and the vtable.
 */

#ifndef _INCLUDED_FUSE_METHODS_H
#define _INCLUDED_FUSE_METHODS_H

#include "common.h"

#include <fuse/fuse_lowlevel.h>

#include "trace.h"


TRACE_DECLARE(method)
#define method_trace(...) TRACE(method,__VA_ARGS__)
#define method_trace_indent() TRACE_INDENT(method)
#define method_trace_dedent() TRACE_DEDENT(method)

TRACE_DECLARE(read)
#define read_trace(...) TRACE(read,__VA_ARGS__)
#define read_trace_indent() TRACE_INDENT(read)
#define read_trace_dedent() TRACE_DEDENT(read)


/* fsfuse fuse methods vtable */
extern struct fuse_lowlevel_ops fuse_methods;


extern void fsfuse_init (void *userdata, struct fuse_conn_info *conn);

extern void fsfuse_destroy (void *userdata);

extern void fsfuse_lookup (fuse_req_t req, fuse_ino_t parent, const char *name);

extern void fsfuse_forget (fuse_req_t req, fuse_ino_t ino, unsigned long nlookup);

extern void fsfuse_getattr (fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);

extern void fsfuse_setattr (fuse_req_t req, fuse_ino_t ino, struct stat *attr, int to_set, struct fuse_file_info *fi);

extern void fsfuse_readlink (fuse_req_t req, fuse_ino_t ino);

extern void fsfuse_mknod (fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, dev_t rdev);

extern void fsfuse_mkdir (fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode);

extern void fsfuse_unlink (fuse_req_t req, fuse_ino_t parent, const char *name);

extern void fsfuse_rmdir (fuse_req_t req, fuse_ino_t parent, const char *name);

extern void fsfuse_symlink (fuse_req_t req, const char *link, fuse_ino_t parent, const char *name);

extern void fsfuse_rename (fuse_req_t req, fuse_ino_t parent, const char *name, fuse_ino_t newparent, const char *newname);

extern void fsfuse_link (fuse_req_t req, fuse_ino_t ino, fuse_ino_t newparent, const char *newname);

extern void fsfuse_open (fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);

extern void fsfuse_read (fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, struct fuse_file_info *fi);

extern void fsfuse_write (fuse_req_t req, fuse_ino_t ino, const char *buf, size_t size, off_t off, struct fuse_file_info *fi);

extern void fsfuse_flush (fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);

extern void fsfuse_release (fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);

extern void fsfuse_fsync (fuse_req_t req, fuse_ino_t ino, int datasync, struct fuse_file_info *fi);

extern void fsfuse_opendir (fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);

extern void fsfuse_readdir (fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, struct fuse_file_info *fi);

extern void fsfuse_releasedir (fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);

extern void fsfuse_fsyncdir (fuse_req_t req, fuse_ino_t ino, int datasync, struct fuse_file_info *fi);

extern void fsfuse_statfs (fuse_req_t req, fuse_ino_t ino);

extern void fsfuse_access (fuse_req_t req, fuse_ino_t ino, int mask);

extern void fsfuse_create (fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, struct fuse_file_info *fi);

extern void fsfuse_bmap (fuse_req_t req, fuse_ino_t ino, size_t blocksize, uint64_t idx);


#endif /* _INCLUDED_FUSE_METHODS_H */
