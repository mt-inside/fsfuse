/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * readdir() implementation.
 */

#include "common.h"

#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "direntry.h"
#include "fuse_methods.h"


/* from fuse_lowlevel.h:
 * "From the 'stbuf' argument the st_ino field and bits 12-15 of the st_mode
 * field are used.  The other fields are ignored".
 * Bytes 12-15 are 010000 - 0100000 (0x1000 - 0x8000) which represent node type
 * (file / dir / etc). Mode flags are ignored.
 */

static void dirbuf_add (
    fuse_req_t req,
    char **buf,
    size_t *size,
    direntry_t *de,
    const char *name
)
{
    struct stat stats;
    size_t old_size = *size;


    name = name ? name : direntry_get_name(de);

    /* Saw this trick (to determine, a priori, the exact size this entry will
     * need) in the fuse_lowlevel example. If it stops working, go back to
     * standard: try, compare return to remaining size, realloc up by MOD on
     * fail. */
    *size += fuse_add_direntry(req, NULL, 0, name, NULL, 0);
    *buf = realloc(*buf, *size);

    direntry_de2stat(de, &stats);
    fuse_add_direntry(req, *buf + old_size, *size - old_size, name, &stats, *size);

    method_trace("dirbuf_adding: inode %lu \"%s\" at %zu size %zu buf size %zu\n",
        stats.st_ino, name, old_size, *size - old_size, *size);
}

/* The FUSE docs assert that readdir() will only be called on existing, valid
 * directories, so there's no need to check for the existence / type of the
 * direntry at path
 */
void fsfuse_readdir (fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, struct fuse_file_info *fi)
{
    int rc = 0;
    direntry_t *de, *parent, *child, *old_child;
    char *buf = NULL; size_t bufsize = 0;


    NOT_USED(size);
    NOT_USED(off);
    NOT_USED(fi);

    method_trace("fsfuse_readir(ino %lu, size %zu, offset %lu)\n", ino, size, off);
    method_trace_indent();


    rc = direntry_get_by_inode(ino, &de);
    if (!rc)
    {
        if (direntry_get_type(de) != listing_type_DIRECTORY) rc = ENOTDIR;

        if (!rc)
        {
            dirbuf_add(req, &buf, &bufsize, de, ".");

            /* NB: we always used to use NULL struct stat for "." and ".." and got
             * away with it fine. We could consider carrying on doing that here, as
             * fetching them could be a pita */
            parent = direntry_get_parent(de);

            if (parent)
            {
                dirbuf_add(req, &buf, &bufsize, parent, "..");
                direntry_delete(CALLER_INFO parent);
            }
            else
            {
                dirbuf_add(req, &buf, &bufsize, de, "..");
            }

            direntry_ensure_children(de);
            child = direntry_get_first_child(de);
            while (child)
            {
                dirbuf_add(req, &buf, &bufsize, child, NULL);

                old_child = child;
                child = direntry_get_next_sibling(child);
                direntry_delete(CALLER_INFO old_child);
            }
        }

        direntry_delete(CALLER_INFO de);
    }

    method_trace_dedent();


    if (!rc)
    {
        if ((unsigned)off < bufsize)
        {
            assert(!fuse_reply_buf(req, buf + off, MIN(bufsize - off, size)));
        }
        else
        {
            assert(!fuse_reply_buf(req, NULL, 0));
        }
    }
    else
    {
        assert(!fuse_reply_err(req, rc));
    }

    free(buf);
}
