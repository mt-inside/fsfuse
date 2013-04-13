/*
 * Copyright (C) 2008-2013 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * statfs() implementation.
 */

#include "common.h"

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "fuse_methods.h"

#include "indexnodes.h"
#include "parser.h"


void fsfuse_statfs (fuse_req_t req, fuse_ino_t ino)
{
    indexnodes_t *ins = ((fsfuse_ctxt_t *)fuse_req_userdata(req))->indexnodes;
    unsigned long files, bytes, bytes_total = 0;
    struct statvfs stvfs;
    int rc = 1;
    const char *url;
    indexnodes_list_t *list;
    indexnodes_iterator_t *iter;
    indexnode_t *in;


    NOT_USED(ino);

    method_trace("fsfuse_statfs(ino %lu)\n", ino);
    method_trace_indent();

    memset(&stvfs, 0, sizeof(struct statvfs));
    stvfs.f_bsize   = FSFUSE_BLKSIZE;
    stvfs.f_frsize  = FSFUSE_BLKSIZE;        /* Ignored by fuse */
    stvfs.f_flag    = ST_RDONLY | ST_NOSUID; /* Ignored by fuse */
    stvfs.f_namemax = ULONG_MAX;

    /* TODO: these should happen in parallel. Some kind of fetcher_thread_pool? */
    list = indexnodes_get(CALLER_INFO ins);
    for (iter = indexnodes_iterator_begin(list);
         !indexnodes_iterator_end(iter);
         iter = indexnodes_iterator_next(iter))
    {
        in = indexnodes_iterator_current(iter);
        url = indexnode_make_url(in, "stats", "");

        if (parser_tryfetch_stats(url, &files, &bytes))
        {
            stvfs.f_files += files;
            bytes_total += bytes;
        }

        indexnode_delete(CALLER_INFO in);
        free_const(url);
    }
    stvfs.f_blocks = bytes_total / stvfs.f_bsize;
    indexnodes_iterator_delete(iter);
    indexnodes_list_delete(list);


    method_trace_dedent();


    if (!rc)
    {
        assert(!fuse_reply_statfs(req, &stvfs));
    }
    else
    {
        assert(!fuse_reply_err(req, rc));
    }
}
