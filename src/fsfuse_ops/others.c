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

int fsfuse_open ( const char *path,
                  struct fuse_file_info *fi )
{
    /* not doing anything special here atm (future posibilities include
     * pre-fetching (incl. parallel pre-fetching in another thread))
     * For now, simply check the existance of path, and check permissions */
    int rc;
    direntry_t *de;


    /* get the direntry first so that if we try to write a non-existant file,
     * the non-existance is complained about over the write flag. Is there a
     * defined order of precidence for this kind of thing anywhere? Lower error
     * code first maybe? */
    rc = direntry_get(path, &de);

    if (!rc)
    {
        if ((fi->flags & 3) != O_RDONLY) rc = -EROFS;
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
        switch (de->type)
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
