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


/* The FUSE docs assert that readdir() will only be called on existing, valid
 * directories, so there's no need to check for the existence / type of the
 * direntry at path
 */
int fsfuse_readdir (const char *path,
                    void *buf,
                    fuse_fill_dir_t filler,
                    off_t offset,
                    struct fuse_file_info *fi)
{
    int rc = 0;
    direntry_t *first_child, *child;
    struct stat *st;


    NOT_USED(offset);
    NOT_USED(fi);

    method_trace("fsfuse_readir(%s)\n", path);
    method_trace_indent();

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    rc = path_get_children(path, &first_child);
    if (!rc)
    {
        child = first_child;
        while (child)
        {
            if (direntry_get_base_name(child) && direntry_get_path(child))
            {
                st = (struct stat *)malloc(sizeof(struct stat));
                direntry_de2stat(st, child);

                filler(buf, direntry_get_base_name(child), st, 0);
                free(st);
            }

            child = direntry_get_next_sibling(child);
        }

        direntry_delete_list(first_child);
    }

    method_trace_dedent();


    return rc;
}
