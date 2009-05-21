/*
 * readdir() implementation.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#include <fuse.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "common.h"
#include "direntry.h"
#include "fsfuse_ops/readdir.h"
#include "fsfuse_ops/others.h"
#include "fetcher.h"


int fsfuse_readdir (const char *path,
                    void *buf,
                    fuse_fill_dir_t filler,
                    off_t offset,
                    struct fuse_file_info *fi)
{
    int rc = 0;
    direntry_t *de, *first_child, *child;
    struct stat *st;


    NOT_USED(offset);
    NOT_USED(fi);

    method_trace("fsfuse_readir(%s)\n", path);
    method_trace_indent();

    rc = direntry_get(path, &de);

    if (!rc)
    {
        if (direntry_get_type(de) != direntry_type_DIRECTORY) rc = -ENOTDIR;

        if (!rc)
        {
            filler(buf, ".", NULL, 0);
            filler(buf, "..", NULL, 0);

            rc = direntry_get_children(de, &first_child);
            if (!rc)
            {
                child = first_child;
                while (child)
                {
                    if (child->base_name && child->path)
                    {
                        st = (struct stat *)malloc(sizeof(struct stat));
                        direntry_de2stat(st, child);

                        filler(buf, child->base_name, st, 0);
                        free(st);
                    }

                    child = direntry_get_next_sibling(child);
                }

                direntry_delete_list(first_child);
            }
        }

        direntry_delete(CALLER_INFO de);
    }

    method_trace_dedent();


    return rc;
}
