/*
 * Definition of the vtable and some methods.
 *
 * Copyright (C) Matthew Turner 2008-2010. All rights reserved.
 *
 * $Id: fsfuse_ops.h 513 2010-03-08 22:27:27Z matt $
 */

#include "common.h"

#include <fuse/fuse_lowlevel.h>

#include "fsfuse_ops/fsfuse_ops.h"


TRACE_DEFINE(method)


static void fsfuse_init (void *userdata, struct fuse_conn_info *conn)
{
    NOT_USED(userdata);
    NOT_USED(conn);

    method_trace("fsfuse_init()\n");
    method_trace_indent();

    method_trace(
        "proto_major: %u, proto_minor: %u, "
        "async_read: %u, max_write: %u, max_readahead: %u\n",
        conn->proto_major,
        conn->proto_minor,
        conn->async_read,
        conn->max_write,
        conn->max_readahead
    );

    method_trace_dedent();


    /* No reply */
}

static void fsfuse_destroy (void *userdata)
{
    NOT_USED(userdata);

    method_trace("fsfuse_destroy()\n");


    /* No reply */
}


struct fuse_lowlevel_ops fsfuse_ops =
{
    &fsfuse_init,        /* init */
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
    NULL                 /* flock */
#endif /* FUSE_USE_VERSION >= 29 */
#endif /* FUSE_USE_VERSION >= 28 */
#endif /* FUSE_USE_VERSION >= 26 */
#endif /* FUSE_USE_VERSION >= 25 */
};
