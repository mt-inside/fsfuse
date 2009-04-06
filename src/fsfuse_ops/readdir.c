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
#include "fetcher.h"


int fsfuse_readdir (const char *path,
                    void *buf,
                    fuse_fill_dir_t filler,
                    off_t offset,
                    struct fuse_file_info *fi)
{
    int rc = 0;
    direntry_t *de, *child;
    struct stat *st;


    NOT_USED(offset);
    NOT_USED(fi);

    method_trace("fsfuse_readir(%s)\n", path);
    method_trace_indent();

    rc = direntry_get_with_children(path, &de);

    if (!rc)
    {
        assert(de);

        filler(buf, ".", NULL, 0);
        filler(buf, "..", NULL, 0);

        /* TODO: write an iterator for de trees */
        child = de->children;
        while (child)
        {
            if (child->base_name && child->path)
            {
                st = (struct stat *)malloc(sizeof(struct stat));
                direntry_de2stat(st, child);

                filler(buf, child->base_name, st, 0);
                free(st);
            }

            child = child->next;
        }

        direntry_delete_with_children(de);
    }
    else
    {
        assert(!de);
    }


    method_trace_dedent();


    return rc;
}
