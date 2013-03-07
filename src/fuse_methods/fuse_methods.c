/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Definition of the vtable and some methods.
 */

#include "common.h"


#include "fuse_methods.h"


TRACE_DEFINE(method)


struct fuse_lowlevel_ops fuse_methods =
{
    .init = &fsfuse_init,        /* init */
    &fsfuse_destroy,     /* destroy */
    &fsfuse_lookup,      /* lookup */
    &fsfuse_forget,      /* forget */
    &fsfuse_getattr,     /* getattr */
    &fsfuse_setattr,     /* setattr */
    &fsfuse_readlink,    /* readlink */
    &fsfuse_mknod,       /* mknod */
    &fsfuse_mkdir,       /* mkdir */
    &fsfuse_unlink,      /* unlink */
    &fsfuse_rmdir,       /* rmdir */
    &fsfuse_symlink,     /* symlink */
    &fsfuse_rename,      /* rename */
    &fsfuse_link,        /* link */
    &fsfuse_open,        /* open */
    &fsfuse_read,        /* read */
    &fsfuse_write,       /* write */
    &fsfuse_flush,       /* flush */
    &fsfuse_release,     /* release */
    &fsfuse_fsync,       /* fsync */
    &fsfuse_opendir,     /* opendir */
    &fsfuse_readdir,     /* readdir */
    &fsfuse_releasedir,  /* releasedir */
    &fsfuse_fsyncdir,    /* fsyncdir */
    &fsfuse_statfs,      /* statfs */
    NULL,                /* setxattr */
    NULL,                /* getxattr */
    NULL,                /* listxattr */
    NULL,                /* removexattr */
#if FUSE_USE_VERSION >= 25
    &fsfuse_access,      /* access */
    &fsfuse_create,      /* create */
#if FUSE_USE_VERSION >= 26
    NULL,                /* getlk */
    NULL,                /* setlk */
    &fsfuse_bmap,        /* bmap */
#if FUSE_USE_VERSION >= 28
    NULL,                /* ioctl */
    NULL,                /* poll */
#if FUSE_USE_VERSION >= 29
    NULL,                /* write_buf */
    NULL,                /* retrieve_reply */
    NULL,                /* forget_multi */
    NULL,                /* flock */
    //NULL,                /* mmap (CUSE only atm) */
    //NULL                 /* munmap (CUSE only atm) */
#endif /* FUSE_USE_VERSION >= 29 */
#endif /* FUSE_USE_VERSION >= 28 */
#endif /* FUSE_USE_VERSION >= 26 */
#endif /* FUSE_USE_VERSION >= 25 */
};
