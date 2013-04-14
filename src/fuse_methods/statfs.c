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
#include <limits.h>
#include <string.h>
#include <stdlib.h>

#include "fuse_methods.h"

#include "fetcher.h"
#include "indexnodes.h"
#include "parser.h"


void fsfuse_statfs (fuse_req_t req, fuse_ino_t ino)
{
    indexnodes_t *ins = ((fsfuse_ctxt_t *)fuse_req_userdata(req))->indexnodes;
    unsigned long files, bytes, bytes_total = 0;
    struct statvfs stvfs;
    int rc = 1;
    parser_t *parser;
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

    /* TODO: these should happen in parallel. Some kind of fetcher_thread_pool?
     * Or, make fetching async. THIS IS GOOD IDEA
     * 1) Make fetch() look async by having it take a
     *   continuation callback that it runs when it's done, which the rest of
     *   the function goes in. Of course as it stands the call to fetch() will
     *   block this thread while the download happens. This is fine for most
     *   operations because they only want to do one fetch (e.g. read a file,
     *   list a dir)
     * 2) Make fetch() actually async (i.e. fire off a new thread to do the
     *   fetch and return instantly). The callback will have to lock the place
     *   it's storing the totals and keep a count of the number of results it's
     *   expecting so it knows when it has all the data.
     * 3) Get said threads from a pool.
     * 4) If fetch() gets threads from a pool, that technically makes
     *   download_thread_pool redundant. Even though dt calls fetch(), ideally
     *   it only does so once and it can still block whatever thread fetch()
     *   ends up on because that's the thread that dt's cbs will run on.
     *   I think we're onto something here. dtp does little but start threads
     *   now (it was never a pool), so fetcher can do that, with the added bonus
     *   that this happens in parallel. Opening a file for read should make a
     *   new downloader() which is basically the dt - the thing with the chunk
     *   list etc. However the new thread comes from fetch(), not from
     *   downloader/dt.
     *   */
    list = indexnodes_get(CALLER_INFO ins);
    for (iter = indexnodes_iterator_begin(list);
         !indexnodes_iterator_end(iter);
         iter = indexnodes_iterator_next(iter))
    {
        parser = parser_new();
        in = indexnodes_iterator_current(iter);
        url = indexnode_make_url(in, "stats", "");
        rc = fetch(
            url,
            NULL, NULL,
            (fetcher_body_cb_t)&parser_consumer, (void *)parser,
            0,
            NULL
        );

        if (!rc)
        {
            parser_tryget_stats(parser, &files, &bytes);
            if (!rc)
            {
                stvfs.f_files += files;
                bytes_total += bytes;
            }
        }

        indexnode_delete(CALLER_INFO in);
        free_const(url);
        parser_delete(parser);
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
