/*
 * Thread pool for downloading of files.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

/* A single thread is used for downloading one file, allowing various
 * optimisations. E.g. it can stream a file, and pass of chunks to various
 * read() threads as the requests come in. This is all done with one HTTP
 * request and one TCP connection.
 * Currently, we simply spawn a new thread for every new file. In time, this
 * naive implementation may turn into a read thread pool.
 * Each thread has a list of chunks (byte ranges) that are requested from that
 * file.
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <inttypes.h>

#include "common.h"
#include "locks.h"
#include "config.h"
#include "direntry.h"
#include "download_thread_pool.h"
#include "download_thread.h"
#include "fetcher.h"
#include "indexnode.h"
#include "queue.h"
#if FEATURE_PROGRESS_METER
#include "progress.h"
#endif


TRACE_DEFINE(dtp)

typedef struct _thread_list_item_t
{
    thread_t *thread;
    TAILQ_ENTRY(_thread_list_item_t) next;
} thread_list_item_t;

/* List of threads we've spawned - this is the thread pool */
TAILQ_HEAD(, _thread_list_item_t) thread_list;
rw_lock_t *thread_list_lock = NULL;


static void thread_end_cb (thread_t *t);
static thread_t *tp_thread_get (direntry_t *de);


int thread_pool_init (void)
{
    dtp_trace("thread_pool_init()\n");

    TAILQ_INIT(&thread_list);
    thread_list_lock = rw_lock_new();


    return 0;
}

void thread_pool_finalise (void)
{
    dtp_trace("thread_pool_finalise()\n");

    rw_lock_delete(thread_list_lock);
}

/* add a request for a range ("chunk") of a file to the threadpool downloader.
 * This finds / creates the thread that's dealing with that file and adds the
 * chunk to its download list.
 * TODO: when we move to a pool, we need to cope with no free threads here and
 * return EBUSY or block */
/* EXECUTES IN THREAD: read() */
void thread_pool_chunk_add (direntry_t *de,
                            off_t start,
                            off_t end,
                            char *buf,
                            chunk_done_cb_t cb,
                            void *ctxt          )
{
    thread_t *thread;


    dtp_trace("thread_pool_chunk_add(hash==%s, start==%#x, end==%#x)\n",
              direntry_get_hash(de),
              start,
              end );
    dtp_trace_indent();


    rw_lock_wlock(thread_list_lock);

    thread = tp_thread_get(de);

    if (!thread)
    {
        dtp_trace("no thread found - creating a new one\n");
        thread = download_thread_new(de, &thread_end_cb);
    }

    rw_lock_wunlock(thread_list_lock);


    download_thread_chunk_add(thread, start, end, buf, cb, ctxt);

    dtp_trace_dedent();
}


static void thread_end_cb (thread_t *t)
{
    thread_list_item_t *item, *item_temp;

    /* TODO: is this a bit O(n^2)? */
    TAILQ_FOREACH_SAFE(item, &thread_list, next, item_temp)
    {
        if (item->thread == t)
        {
            TAILQ_REMOVE(&thread_list, item, next);
        }
    }
}

/* Return the thread currently dealing with hash, or NULL */
/* EXECUTES IN THREAD: read()
 * Accesses thread_list - mutex needed! */
static thread_t *tp_thread_get (direntry_t *de)
{
    thread_list_item_t *ti = NULL;


    rw_lock_rlock(thread_list_lock);

    TAILQ_FOREACH(ti, &thread_list, next)
    {
        if (download_thread_is_for(ti->thread, de)) break;
    }

    rw_lock_runlock(thread_list_lock);


    return ti ? ti->thread : NULL;
}
