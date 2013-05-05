/*
 * Copyright (C) 2008-2013 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Downloader class.
 * Downloaders download files.
 * A downloader currently has a thread for doing the downloading (though it's
 * proposed that the thread comes from fetcher in future).
 * These is one thread per open()ed file handle. The idea is that if you've
 * open()ed the file twice you're probably going to read() from different points
 * so all the streaming optimisations go out of the window. It also makes the
 * usage pattern more transparent to the indexndoe, for whatever that's worth.
 * NB that userspace can still use a filehandle from multiple threads, or dup()
 * it or fork(), so we could still get parallel requests to the downloader hence
 * it still needs to be threadsafe.
 *
 * TODO: I should be an actor!
 * TODO: Currently the thread finishes at some random point (maybe when the
 * file's been read to the end?). It should live for the open duration of the
 * file; release should call downloader_delete()
 */

#include "common.h"

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "downloader.h"

#include "config_manager.h"
#include "config_reader.h"
#include "direntry.h"
#include "fetcher.h"
#include "locks.h"
#include "queue.h"
#include "string_buffer.h"


TRACE_DEFINE(downloader)


typedef struct _chunk_t
{
    off_t start; /* inclusive */
    off_t end;   /* exclusive */
    size_t bytes_used; /* no. bytes currently populated (starting at start) */
    char *buf;
    chunk_done_cb_t cb;
    void *ctxt;

    TAILQ_ENTRY(_chunk_t) entries;
} chunk_t;

typedef struct
{
    off_t start; /* inclusive */
    off_t end;   /* exclusive */
    char *buf;
} buf_t;

struct _downloader_t
{
    direntry_t *de; /* TODO: should this be a listing_t */
    /* TODO: singly linked list is wrong. expected case is that new items go on the
     * end, giving O(n^2) running time. Need a doubly-linked list and an end
     * pointer */
    TAILQ_HEAD(, _chunk_t) chunk_list;
    unsigned chunk_list_count;
    pthread_cond_t chunk_list_cond;
    pthread_mutex_t chunk_list_mutex;
    off_t download_offset;
    pthread_t pthread_id;
    chunk_t *current_chunk; /* needs to be kept separate because it may not be
                               at the head of the list at any time */
    int seek;               /* flags indicating why we bailed */
    int timed_out;
};


static void thread_delete (downloader_t *t);

static chunk_t *chunk_new (
    off_t start,
    off_t end,
    char *buf,
    chunk_done_cb_t rc,
    void *ctxt
);
static chunk_t *chunk_get_next (downloader_t *thread);

static void *downloaderead_main (void *arg);
static void chunk_list_empty (downloader_t *thread, int rc);
static int thread_pool_consumer (void *ctxt, void *data, size_t len);
static void signal_read_thread (chunk_t *chunk, int rc, size_t size);


/* Create a new thread to download <hash> (and start the download process). */
downloader_t *downloader_new (direntry_t *de)
{
    downloader_t *t = (downloader_t *)calloc(1, sizeof(downloader_t));
    pthread_attr_t attr;


    /* thread attributes */
    pthread_attr_init(&attr);
    /* setting the thread's detached state means that as soon as it exits it's
     * completely cleaned up. Otherwise some resources would hang around until
     * pthread_join() is used to read its return code - which we don't
     * (currently) want to do. */
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);


    /* fill in the thread's details */
    t->de = direntry_copy(CALLER_INFO de);
    TAILQ_INIT(&t->chunk_list);
    pthread_cond_init(&(t->chunk_list_cond), NULL);
    pthread_mutex_init(&(t->chunk_list_mutex), NULL);


    /* make the thread */
    /* is this safe? presumably the new thread could read pthread_id before
     * it's been written by this thread. Do we, in fact, care what it's id is
     * at all? */
    assert(!pthread_create(&(t->pthread_id), &attr, &downloaderead_main, (void *)t));

    pthread_attr_destroy(&attr);


    return t;
}

static void thread_delete (downloader_t *t)
{
    assert(!t->current_chunk);
    assert(TAILQ_EMPTY(&t->chunk_list));

    direntry_delete(CALLER_INFO t->de);
    pthread_cond_destroy(&t->chunk_list_cond);
    pthread_mutex_destroy(&t->chunk_list_mutex);

    free(t);
}

/* Add a chunk to the chunk list for an individual thread.
 * These go in in order - start of file to end of file */
/* Note that start and end can be the same value indicating a 0 byte range */
void downloader_chunk_add (downloader_t *thread,
                                off_t start, /* inclusive */
                                off_t end,   /* exclusive */
                                char *buf,
                                chunk_done_cb_t cb,
                                void *ctxt          )
{
    chunk_t *c = NULL, *curr;


    assert(start <= end);
    assert(start <= direntry_get_size(thread->de));
    assert(end   <= direntry_get_size(thread->de));

    c = chunk_new(start, end, buf, cb, ctxt);


    pthread_mutex_lock(&(thread->chunk_list_mutex));

    /* this new chunk needs to go into the list in order. Chunks cannot be
     * aggregated because they all get passed off to separate buffers, but
     * having them in order optimises the sequential read case. */
    /* overlapping chunks are a bit of an odd request, but should
     * just cause a seek and work OK */
    if (TAILQ_EMPTY(&thread->chunk_list) || c->start <= TAILQ_FIRST(&thread->chunk_list)->start)
    {
        TAILQ_INSERT_HEAD(&thread->chunk_list, c, entries);
    }
    else
    {
        TAILQ_FOREACH(curr, &thread->chunk_list, entries)
        {
            if (c->start <= curr->start)
            {
                TAILQ_INSERT_BEFORE(curr, c, entries);
                break;
            }
        }
        if (!curr)
        {
            TAILQ_INSERT_TAIL(&thread->chunk_list, c, entries);
        }
    }

    /* increment the chunk list count */
    thread->chunk_list_count++;
    pthread_cond_signal(&thread->chunk_list_cond);

    pthread_mutex_unlock(&(thread->chunk_list_mutex));
}

static chunk_t *chunk_new (
    off_t start,
    off_t end,
    char *buf,
    chunk_done_cb_t cb,
    void *ctxt
)
{
    chunk_t *c = (chunk_t *)calloc(1, sizeof(chunk_t));


    assert(c);

    c->start = start;
    c->end   = end;
    c->buf   = buf;
    c->cb    = cb;
    c->ctxt  = ctxt;


    return c;
}

static void chunk_delete (chunk_t *chunk)
{
    /* caller is responsible for destroying and freeing ctxt */
    free(chunk);
}

static chunk_t *chunk_get_next (downloader_t *thread)
{
    chunk_t *chunk;
    int rc = 0;
    struct timeval tv;
    struct timespec ts;
    config_reader_t *config = config_get_reader();


    /* wait for a chunk on the list */
    downloader_trace("chunk_get_next() waiting %u seconds for new chunk...\n",
              config_timeout_chunk(config));
    downloader_trace_indent();


    gettimeofday(&tv, NULL);
    ts.tv_sec = tv.tv_sec + config_timeout_chunk(config);
    ts.tv_nsec = (tv.tv_usec + 1) * 1000;

    pthread_mutex_lock(&thread->chunk_list_mutex);
    while (!thread->chunk_list_count && rc == 0)
    {
        rc = pthread_cond_timedwait(&thread->chunk_list_cond,
                                    &thread->chunk_list_mutex,
                                    &ts);
    }
    if (rc == 0) thread->chunk_list_count--;
    pthread_mutex_unlock(&thread->chunk_list_mutex);

    if (rc == ETIMEDOUT)
    {
        /* timed out, abort */
        chunk = NULL;
        downloader_trace("timed out\n");
    }
    else if (rc == 0)
    {
        downloader_trace("Downloader thread for %s woken up! "
                  "Chunks remaining: %d\n",
                  direntry_get_name(thread->de),
                  thread->chunk_list_count);


        pthread_mutex_lock(&(thread->chunk_list_mutex));

        chunk = TAILQ_FIRST(&thread->chunk_list);
        /* there must be a chunk, because we just came out of the semaphore */
        assert(chunk);


        /* remove the chunk from the list */
        TAILQ_REMOVE(&thread->chunk_list, chunk, entries);

        pthread_mutex_unlock(&(thread->chunk_list_mutex));
    }
    else
    {
        downloader_trace("pthread_cond_timedwait(): error\n");
        assert(0);
    }

    config_reader_delete(config);

    downloader_trace_dedent();


    return chunk;
}

/* This is the entry point for new downloader threads. */
/* EXECUTES IN THREAD: downloaderead_main().
 * Accesses a thread - ?no mutex needed */
static void *downloaderead_main (void *arg)
{
    downloader_t *thread = (downloader_t *)arg;
    string_buffer_t *range_buffer;
    listing_t *li;
    char *range_str;
    int rc;
    int first_time = 1;


    downloader_trace("downloaderead_main(file==%s): New downloader thread!\n",
              direntry_get_name(thread->de) );
    downloader_trace_indent();

    /* We never try to guess when we're done with a file completely and call
     * fall straight out of this loop (e.g. when we think we've served the whole
     * file up - we can't tell unless we keep expensive track of it, and what
     * does it matter anyway?)
     * If we've seeked then we must have popped a chunk into current_chunk. The
     * state of the chunk list is irrelevant.
     * If we've not seeked, then we've either just entered this function (just
     * started the file from some position) or we came out of the fetcher
     * cleanly because we consumed the last buffer of the file. In either case,
     * there might be another chunk on the list already or there might be one
     * soon (because we want to start fetching the file from some point, or
     * because we want to go back and fetch a chunk we seeked around), thus we
     * have to wait.
     * This behaviour is fine because we will have signalled all the read()
     * threads for all the chunks we've filled, so if we did cover the whole
     * file FUSE will return the user's read() call, and we just tidy up after
     * a bit of a wait, just like they'd stopped requesting a file half way
     * through.
     * TODO: Maybe need to think about separating the two timeouts - how long we
     * wait for the system to get us another read() chunk when we've returned
     * the last one, and how long we give the user to request something else
     * from the file before we decide they're done with it. The first can
     * probably be quite short, while the latter might need to be quite long to
     * account for very low bit rate streaming. For now I don't see any reason
     * why they can't both be very long (save resource usage) because neither
     * should hold anything important up.
     */
    do
    {
        /* DEBUGGING ======================================================== */
        /* this flag is a nicety, to try to make sure things are sane */
        if (thread->seek)
        {
            /* If we've seeked there must be a chunk in hand */
            assert(thread->current_chunk);
            thread->seek = 0;
        }
        if (first_time)
        {
            assert(!thread->current_chunk);
            first_time = 0;
        }
        /* ================================================================== */

        /* We won't have a chunk if we've just come in here. At all other times
         * we should */
        if (!thread->current_chunk)
        {
            thread->current_chunk = chunk_get_next(thread);

            if (!thread->current_chunk)
            {
                /* timed out */
                break;
            }
        }

        range_buffer = string_buffer_new();
        if (thread->current_chunk->start)
        {
            string_buffer_printf(range_buffer, "%" PRIu64 "-", thread->current_chunk->start);
        }
        range_str = string_buffer_commit(range_buffer);
        thread->download_offset = thread->current_chunk->start;
        downloader_trace("fetching range \"%s\"\n", range_str);

        /* Find alternatives */
        /* TODO: BASE_CLASS() should be universal */
        if (listing_tryget_best_alternative((listing_t *)thread->de, &li))
        {
            fetcher_t *fetcher = fetcher_new(listing_get_href(li));

            /* Get the file */
            rc = fetcher_fetch_body(
                fetcher,
                (fetcher_body_cb_t)&thread_pool_consumer,
                (void *)thread,
                (thread->current_chunk->start) ? range_str : NULL
            );

            listing_delete(CALLER_INFO li);
            fetcher_delete(fetcher);
        }
        else
        {
            rc = EBUSY;
            /* TODO sort out handling of this whole area */
        }

        free(range_str);

    } while (rc == 0);


    pthread_mutex_lock(&(thread->chunk_list_mutex));

    if (rc == 0)
    {
        /* OK */
        /* There should be no more chunks, either in-hand or on the list */
        assert(!thread->current_chunk);
        assert(TAILQ_EMPTY(&thread->chunk_list));
    }
    else
    {
        if (thread->current_chunk)
        {
            signal_read_thread(thread->current_chunk, rc, 0);
            chunk_delete(thread->current_chunk);
            thread->current_chunk = NULL;

            chunk_list_empty(thread, rc);
        }
    }

    pthread_mutex_unlock(&(thread->chunk_list_mutex));


    /* Delete the data structures */
    thread_delete(thread); /* Is this safe here? */

    downloader_trace("downloader_main() returning\n");
    downloader_trace_dedent();


    pthread_exit(NULL);
}

static void chunk_list_empty (downloader_t *thread, int rc)
{
    chunk_t *c;


    while ((c = TAILQ_FIRST(&thread->chunk_list)))
    {
        signal_read_thread(c, rc, 0);
        TAILQ_REMOVE(&thread->chunk_list, TAILQ_FIRST(&thread->chunk_list), entries);
        chunk_delete(c);
    }
}


/* EXECUTES IN THREAD: downloaderead_main(). */
static int thread_pool_consumer (void *ctxt, void *data, size_t len)
{
    buf_t *buf = (buf_t *)malloc(sizeof(buf_t));
    downloader_t *thread = (downloader_t *)ctxt;
    chunk_t *chunk;
    int rc = 1;
    size_t copy_len;


    if (!thread->current_chunk) thread->current_chunk = chunk_get_next(thread);
    if (!thread->current_chunk)
    {
        thread->timed_out = 1;
        goto bail;
    }
    chunk = thread->current_chunk;

    buf->start = thread->download_offset;
    buf->end   = thread->download_offset + len;
    buf->buf   = data;


    while (1)
    {
        downloader_trace("buf: %#x-%#x; chunk: %#x-%#x\n", buf->start, buf->end, chunk->start, chunk->end);

        if (chunk->start == buf->start)
        {
            downloader_trace("chunk ok - continuing stream\n");

            /* How much to copy? */
            copy_len = MIN((buf->end - buf->start), (chunk->end - chunk->start));
            downloader_trace("copy_len == %#x\n", copy_len);

            /* DO IT */
            downloader_trace("doing it: absolute start == %#x, absolute end == %#x\n",
                      buf->start, buf->start + copy_len);
            memcpy(chunk->buf, buf->buf, copy_len);

            chunk->bytes_used += copy_len;

            buf->start += copy_len;
            buf->buf   += copy_len;
            downloader_trace("buf->start now == %#x\n", buf->start);
            chunk->start += copy_len;
            chunk->buf   += copy_len;
            downloader_trace("chunk->start now == %#x\n", buf->start);


            if (buf->start == buf->end && chunk->start == chunk->end)
            {
                downloader_trace("End of buf && chunk\n");

                /* End of chunk stuff
                 * (but don't get a new one because end of buf might mean end
                 * of file) */
                signal_read_thread(chunk, 0, chunk->bytes_used);

                /* free the chunk */
                chunk_delete(chunk);

                /* indicate current chunk is no longer valid, but don't replace
                 * it, as we don't know where the next chunk is going to start
                 * */
                thread->current_chunk = NULL;


                /* end of buf stuff */
                break;
            }

            if (buf->start == buf->end)
            {
                downloader_trace("End of buf\n");
                break;
            }

            if (chunk->start == chunk->end)
            {
                downloader_trace("End of chunk\n");

                /* we've reached the end of this chunk's buffer. signal its
                 * semaphore so that its read() thread can wake and return */
                signal_read_thread(chunk, 0, chunk->bytes_used);

                /* free the chunk */
                chunk_delete(chunk);

                thread->current_chunk = chunk_get_next(thread);
                if (!thread->current_chunk)
                {
                    thread->timed_out = 1;
                    goto bail;
                }
                chunk = thread->current_chunk;
                assert(chunk); /* shouldn't happen - chunk_get_next() should block */
            }
        }
        else
        {
            downloader_trace("chunk not ok - bailing out to seek\n");

            /* haven't consumed the whole buf, so curl will bail out with an
             * error and we end up back in downloaderead_main() */
            thread->seek = 1;
            goto bail;
        }
    }
    downloader_trace("buf->start caught buf->end - finished buf processing loop\n");

    thread->download_offset = buf->end;
    downloader_trace("offset now == %#x\n\n", thread->download_offset);
    rc = 0;


bail:
    free(buf);

    downloader_trace_dedent();


    return rc;
}

static void signal_read_thread (chunk_t *chunk, int rc, size_t size)
{
    downloader_trace("signalling read() thread - err %d, %zu bytes\n", rc, size);

    chunk->cb(chunk->ctxt, rc, size);
}


#if DEBUG
void dump_chunk (chunk_t *chunk)
{
    trace_np("%#x -> %#x -> %#x", chunk->start, chunk->bytes_used, chunk->end);
}

void dump_chunk_list (downloader_t *thread)
{
    chunk_t *curr;
    unsigned i = 0;


    if (TAILQ_EMPTY(&thread->chunk_list))
    {
        trace_np("  <empty>\n");
    }
    else
    {
        TAILQ_FOREACH(curr, &thread->chunk_list, entries)
        {
            trace_np("  [%u] ", i++);
            dump_chunk(curr);
        }
        trace_np("\n");
    }
}
#endif /* DEBUG */
