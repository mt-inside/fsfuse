/*
 * Implementations for misc. filesystem operations.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#include <fuse.h>
#include <errno.h>

#include "common.h"
#include "config.h"
#include "fsfuse_ops/others.h"
#include "trace.h"
#include "direntry.h"


int fsfuse_getattr ( const char *path,
                     struct stat *stbuf )
{
    int rc;
    direntry_t *de;


    trce("fsfuse_getattr(%s)\n", path);
    trace_indent();

    rc = direntry_get(path, &de);

    if (!rc)
    {
        assert(de);

        direntry_de2stat(stbuf, de);
        direntry_delete(de);
    }
    else
    {
        assert(!de);
    }

    trace_dedent();


    return rc;
}

int fsfuse_readlink ( const char *path,
                      char * buf,
                      size_t len )
{
    NOT_USED(buf);
    NOT_USED(len);

    trce("fsfuse_readlink(path==%s). SHOULD NOT HAPPEN\n", path);

    /* We do not currently claim that there are any symlinks in an fsfuse
     * filesystem (although I envisage search and/or multiple file alternatives
     * will be implemented like this in future), so we shouldn't currently be
     * getting any queries about them */
    assert(0);


    return 0; /* success */
}

int fsfuse_open ( const char *path,
                  struct fuse_file_info *fi )
{
    /* not doing anything special here atm (future posibilities include
     * pre-fetching (incl. parallel pre-fetching in another thread))
     * For now, simply check the existance of path, and check permissions */
    int rc;
    direntry_t *de;


    trce("fsfuse_open(path==%s)\n", path);

    rc = direntry_get(path, &de);

    if (!rc)
    {
        /* Ordering below is deliberate - the reverse of our order of presidence
         * for complaining (TODO: which is a guess anyway). */
        if ((fi->flags & 3) != O_RDONLY)                 rc = -EROFS;
        if (direntry_get_type(de) != direntry_type_FILE) rc = -EISDIR;
    }

    direntry_delete(de);


    return rc;
}

int fsfuse_mknod ( const char *path,
                   mode_t mode,
                   dev_t dev         )
{
    trce("fsfuse_mknod(path==%s, mode=%#x, dev=%#x)\n", path, mode, dev);


    return -EROFS;
}

int fsfuse_mkdir ( const char *path,
                   mode_t mode       )
{
    trce("fsfuse_mkdir(path==%s, mode=%#x)\n", path, mode);


    return -EROFS;
}

int fsfuse_unlink ( const char *path )
{
    trce("fsfuse_unlink(path==%s)\n", path);


    return -EROFS;
}

int fsfuse_rmdir ( const char *path )
{
    trce("fsfuse_rmdir(path==%s)\n", path);


    return -EROFS;
}

int fsfuse_symlink ( const char *from,
                     const char *to    )
{
    trce("fsfuse_symlink(from==%s, to==%s)\n", from, to);


    return -EROFS;
}

int fsfuse_rename ( const char *from,
                    const char *to    )
{
    trce("fsfuse_rename(from==%s, to==%s)\n", from, to);


    return -EROFS;
}

int fsfuse_link ( const char *from,
                  const char *to    )
{
    trce("fsfuse_link(from==%s, to==%s)\n", from, to);


    return -EROFS;
}

int fsfuse_chmod ( const char *path,
                   mode_t mode       )
{
    trce("fsfuse_chmod(path==%s, mode=%#x)\n", path, mode);


    return -EROFS;
}

int fsfuse_chown ( const char *path,
                   uid_t user,
                   gid_t group       )
{
    trce("fsfuse_chown(path==%s, user=%d, group=%d)\n", path, user, group);


    return -EROFS;
}

int fsfuse_truncate ( const char *path,
                      off_t offset      )
{
    trce("fsfuse_truncate(path==%s, offset=%ju)\n", path, offset);


    return -EROFS;
}

int fsfuse_write ( const char *path,
                   const char *buf,
                   size_t size,
                   off_t off,
                   struct fuse_file_info *fi )
{
    trce("fsfuse_write(path==%s, size==%zd, off==%ju)\n", path, size, off);

    NOT_USED(buf);
    NOT_USED(off);
    NOT_USED(fi);


    return -EROFS;
}

int fsfuse_fsync ( const char *path,
                   int datasync,
                   struct fuse_file_info *fi )
{
    trce("fsfuse_fsync(path==%s, datasync==%d)\n", path, datasync);

    NOT_USED(fi);


    return -EROFS;
}

int fsfuse_opendir ( const char *path,
                     struct fuse_file_info *fi )
{
    /* For now, simply check the existence of path, and check permissions */
    int rc;
    direntry_t *de;


    trce("fsfuse_opendir(path==%s)\n", path);

    rc = direntry_get(path, &de);

    if (!rc)
    {
        /* can you open a directory for write? The libc function doesn't take
         * any flags... */
        /* Ordering below is deliberate - the reverse of our order of presidence
         * for complaining (TODO: which is a guess anyway). */
        if ((fi->flags & 3) != O_RDONLY)                      rc = -EROFS;
        if (direntry_get_type(de) != direntry_type_DIRECTORY) rc = -ENOTDIR;
    }

    direntry_delete(de);


    return rc;
}

int fsfuse_fsyncdir ( const char *path,
                      int datasync,
                      struct fuse_file_info *fi )
{
    trce("fsfuse_fsyncdir(path==%s, datasync==%d)\n", path, datasync);

    NOT_USED(fi);


    return -EROFS;
}

int fsfuse_create ( const char *path,
                    mode_t mode,
                    struct fuse_file_info *fi )
{
    trce("fsfuse_create(path==%s, mode==%#x)\n", path, mode);

    NOT_USED(fi);


    return -EROFS;
}

int fsfuse_ftruncate ( const char *path,
                       off_t offset,
                       struct fuse_file_info *fi )
{
    trce("fsfuse_ftruncate(path==%s, offset=%ju)\n", path, offset);

    NOT_USED(fi);


    return -EROFS;
}

int fsfuse_access ( const char *path,
                    int mode          )
{
    int rc = 0;
    direntry_t *de;


    trce("fsfuse_access(%s, %o)\n", path, mode);
    trace_indent();

    rc = direntry_get(path, &de);

    if (!rc)
    {
        switch (direntry_get_type(de))
        {
            case direntry_type_DIRECTORY:
                if (mode & ~config_get(config_key_DIR_MODE).int_val)  rc = -EACCES;
                break;
            case direntry_type_FILE:
                if (mode & ~config_get(config_key_FILE_MODE).int_val) rc = -EACCES;
                break;
        }
    }

    direntry_delete(de);
    trace_dedent();


    return rc;
}

/* Totally pointless operation on a pretend filesystem not stored on a block
 * device, which is read-only anyway. I'd like to see anyone swap onto fsfuse,
 * or install a bootloader on it */
int fsfuse_bmap (const char *path,
                 size_t blocksize,
                 uint64_t *idx)
{
    trce("fsfuse_bmap(path==%s, blocksize=%zu, idx==%lu)\n",
         path, blocksize, idx);
    trce("bmap would seem rather pointless... Anything calling it is likely to malfunction pretty quickly\n");
    assert(0); /* bmap? Presumably, someone could call this and then try to
                  write to some "underlying" block device, not that we report
                  one, at which point things will go horribly wrong. */


    return 0;
}
