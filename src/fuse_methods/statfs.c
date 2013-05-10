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


typedef struct
{
    fuse_req_t req;
    struct statvfs stvfs;
    unsigned long bytes_total;
    unsigned cbs_remaining;
} stats_ctxt_t;


static void stats_cb( void *ctxt, unsigned long files, unsigned long bytes )
{
    stats_ctxt_t *stats = (stats_ctxt_t *)ctxt;


    stats->stvfs.f_files += files;
    stats->bytes_total += bytes;
    stats->cbs_remaining--;


    if( stats->cbs_remaining == 0 )
    {
        stats->stvfs.f_blocks = stats->bytes_total / stats->stvfs.f_bsize;

        assert( !fuse_reply_statfs( stats->req, &stats->stvfs ) );

        free( ctxt );
    }
}

void fsfuse_statfs (fuse_req_t req, fuse_ino_t ino)
{
    indexnodes_t *ins = ((fsfuse_ctxt_t *)fuse_req_userdata(req))->indexnodes;
    stats_ctxt_t *ctxt = calloc( 1, sizeof(*ctxt) );
    indexnodes_list_t *list;
    indexnodes_iterator_t *iter;
    indexnode_t *in;


    NOT_USED(ino);

    method_trace("fsfuse_statfs(ino %lu)\n", ino);
    method_trace_indent();

    ctxt->req = req;

    ctxt->stvfs.f_bsize   = FSFUSE_BLKSIZE;
    ctxt->stvfs.f_frsize  = FSFUSE_BLKSIZE;        /* Ignored by fuse */
    ctxt->stvfs.f_flag    = ST_RDONLY | ST_NOSUID; /* Ignored by fuse */
    ctxt->stvfs.f_namemax = ULONG_MAX;

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

    /* TODO: double-looping is yuk, but there's no way to get list size atm.
     * However that data structure's gonna change to a hash thingy anyway */
    for (iter = indexnodes_iterator_begin(list);
         !indexnodes_iterator_end(iter);
         iter = indexnodes_iterator_next(iter))
    {
        in = indexnodes_iterator_current(iter);

        ctxt->cbs_remaining++;

        indexnode_delete(CALLER_INFO in);
    }
    indexnodes_iterator_delete(iter);

    for (iter = indexnodes_iterator_begin(list);
         !indexnodes_iterator_end(iter);
         iter = indexnodes_iterator_next(iter))
    {
        in = indexnodes_iterator_current(iter);

        indexnode_tryget_stats(in, stats_cb, ctxt);

        indexnode_delete(CALLER_INFO in);
    }
    indexnodes_iterator_delete(iter);

    indexnodes_list_delete(list);


    method_trace_dedent();
}
