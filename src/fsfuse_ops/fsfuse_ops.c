#include <fuse.h>

#include "fsfuse_ops/fsfuse_ops.h"


TRACE_DEFINE(method)


static void *fsfuse_init (struct fuse_conn_info *conn)
{
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


    return NULL;
}

static void fsfuse_destroy (void *private_data)
{
    NOT_USED(private_data);

    method_trace("fsfuse_destroy()\n");
}


struct fuse_operations fsfuse_oper =
{
    &fsfuse_getattr,     /* getattr */
    &fsfuse_readlink,    /* readlink */
    NULL,                /* getdir D */
    &fsfuse_mknod,       /* mknod */
    &fsfuse_mkdir,       /* mkdir */
    &fsfuse_unlink,      /* unlink */
    &fsfuse_rmdir,       /* rmdir */
    &fsfuse_symlink,     /* symlink */
    &fsfuse_rename,      /* rename */
    &fsfuse_link,        /* link */
    &fsfuse_chmod,       /* chmod */
    &fsfuse_chown,       /* chown */
    &fsfuse_truncate,    /* truncate */
    NULL,                /* utime D */
    &fsfuse_open,        /* open */
    &fsfuse_read,        /* read */
    &fsfuse_write,       /* write */
    &fsfuse_statfs,      /* statfs */
    &fsfuse_flush,       /* flush */
    &fsfuse_release,     /* release */
    &fsfuse_fsync,       /* fsync */
    NULL,                /* setxattr */
    NULL,                /* getxattr */
    NULL,                /* listxattr */
    NULL,                /* removexattr */
    &fsfuse_opendir,     /* opendir */
    &fsfuse_readdir,     /* readdir */
    &fsfuse_releasedir,  /* releasedir */
    &fsfuse_fsyncdir,    /* fsyncdir */
    &fsfuse_init,        /* init */
    &fsfuse_destroy,     /* destroy */
    &fsfuse_access,      /* access */
    &fsfuse_create,      /* create */
    &fsfuse_ftruncate,   /* ftruncate */
    NULL,                /* fgetattr */
    NULL,                /* lock */
    NULL,                /* utimens */
    &fsfuse_bmap         /* bmap */
};
