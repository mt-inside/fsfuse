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
#include "fetcher.h"
#include "indexnode.h"
#include "queue.h"
#if FEATURE_PROGRESS_METER
#include "progress.h"
#endif
#include "fsfuse_ops/read.h"


TRACE_DEFINE(dtp)


typedef struct _chunk_t
{
    off_t start; /* inclusive */
    off_t end;   /* exclusive */
    size_t bytes_used; /* no. bytes currently populated (starting at start) */
    char *buf;
    void *ctxt;

    TAILQ_ENTRY(_chunk_t) entries;
} chunk_t;

typedef struct
{
    off_t start; /* inclusive */
    off_t end;   /* exclusive */
    char *buf;
} buf_t;

typedef struct _thread_t
{
    direntry_t *de;
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
    pthread_mutex_t mutex;  /* mutex to protect the thread_t object, as multiple
                               threads can be interested in a single file at
                               once, even if only one userspace programme is
                               trying to access it... */

    TAILQ_ENTRY(_thread_t) entries;
} thread_t;


/* List of threads we've spawned - this is the thread pool */
TAILQ_HEAD(, _thread_t) thread_list;
rw_lock_t *thread_list_lock = NULL;


/* read() thread */
static thread_t *tp_thread_get (const char *hash);
static thread_t *tp_thread_new (direntry_t *de);
static void thread_delete (thread_t *t);

static chunk_t *chunk_new (off_t start, off_t end, char *buf, void *ctxt);
static chunk_t *chunk_get_next (thread_t *thread);
static void thread_chunk_list_add_chunk (thread_t *thread,
                                         off_t start,
                                         off_t end,
                                         char *buf,
                                         void *ctxt        );

/* thread pool thread */
static void *downloader_thread_main (void *arg);
static void chunk_list_empty (thread_t *thread, int rc);
static size_t thread_pool_consumer (void *buf, size_t size, size_t nmemb, void *userp);
static void signal_read_thread (void *ctxt, int rc);

/* debugging */
static void dump_chunk (chunk_t *chunk);
static void dump_chunk_list (thread_t *thread);


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
 * Returns a semaphore that will block until buf has been filled by one of the
 * threads in the pool.
 * TODO: when we move to a pool, we need to cope with no free threads here and
 * return EBUSY or block */
/* EXECUTES IN THREAD: read() */
void thread_pool_chunk_add (direntry_t *de,
                            off_t start,
                            off_t end,
                            char *buf,
                            void *ctxt      )
{
    thread_t *thread = tp_thread_get(direntry_get_hash(de));


    dtp_trace("thread_pool_chunk_add(path==%s, hash==%s, start==%#x, end==%#x)\n",
              direntry_get_path(de),
              direntry_get_hash(de),
              start,
              end );
    dtp_trace_indent();

    if (!thread)
    {
        dtp_trace("no thread found - creating a new one\n");
        thread = tp_thread_new(de);
    }

    thread_chunk_list_add_chunk(thread, start, end, buf, ctxt);

    dtp_trace_dedent();
}

/* Return the thread currently dealing with hash, or NULL */
/* EXECUTES IN THREAD: read()
 * Accesses thread_list - mutex needed! */
static thread_t *tp_thread_get (const char *hash)
{
    thread_t *t = NULL;


    rw_lock_rlock(thread_list_lock);

    TAILQ_FOREACH(t, &thread_list, entries)
    {
        if (!strcmp(hash, direntry_get_hash(t->de))) break;
    }

    rw_lock_runlock(thread_list_lock);


    return t;
}

/* Create a new thread to download <hash> (and start the download process).
 * TODO: when we move to a pool, this will have to grab a thread from the pool
 * and commondeer it */
/* EXECUTES IN THREAD: read().
 * Don't need to take the thread's sem, because it's not on the list, so nothing
 * can get hold of it. TODO: race condition - two read() threads make threads
 * for the same file at the same time */
static thread_t *tp_thread_new (direntry_t *de)
{
    thread_t *t = (thread_t *)calloc(1, sizeof(thread_t));
    pthread_attr_t attr;


    /* thread attributes */
    pthread_attr_init(&attr);
    /* setting the thread's detached state means that as soon as it exits it's
     * completely cleaned up. Otherwise some resources would hang around until
     * pthread_join() is used to read its return code - which we don't
     * (currently) want to do. */
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);


    /* fill in the thread's details */
    direntry_post(CALLER_INFO de);
    t->de = de;
    TAILQ_INIT(&t->chunk_list);
    pthread_cond_init(&(t->chunk_list_cond), NULL);
    pthread_mutex_init(&(t->chunk_list_mutex), NULL);
    pthread_mutex_init(&(t->mutex), NULL);


    /* make the thread */
    /* is this safe? presumably the new thread could read pthread_id before
     * it's been written by this thread. Do we, in fact, care what it's id is
     * at all? */
    assert(!pthread_create(&(t->pthread_id), &attr, &downloader_thread_main, (void *)t));

    pthread_attr_destroy(&attr);

    /* add to the thread list */
    rw_lock_wlock(thread_list_lock);
    TAILQ_INSERT_HEAD(&thread_list, t, entries);
    rw_lock_wunlock(thread_list_lock);


    return t;
}

static void thread_delete (thread_t *t)
{
    assert(!t->current_chunk);
    assert(TAILQ_EMPTY(&t->chunk_list));

    pthread_mutex_unlock(&(t->mutex));

    direntry_delete(CALLER_INFO t->de);
    pthread_cond_destroy(&t->chunk_list_cond);
    pthread_mutex_destroy(&t->chunk_list_mutex);
    pthread_mutex_destroy(&t->mutex);

    TAILQ_REMOVE(&thread_list, t, entries);

    free(t);
}

static chunk_t *chunk_new (off_t start, off_t end, char *buf, void *ctxt)
{
    chunk_t *c = (chunk_t *)calloc(1, sizeof(chunk_t));


    assert(c);

    c->start = start;
    c->end   = end;
    c->buf   = buf;
    c->ctxt  = ctxt;


    return c;
}

static void chunk_delete (chunk_t *chunk)
{
    /* caller is responsible for destroying and freeing ctxt */
    free(chunk);
}

static chunk_t *chunk_get_next (thread_t *thread)
{
    chunk_t *chunk;
    int rc = 0;
    struct timeval tv;
    struct timespec ts;


    /* wait for a chunk on the list */
    dtp_trace("chunk_get_next() waiting %u seconds for new chunk...\n",
              config_timeout_chunk);
    dtp_trace_indent();


    gettimeofday(&tv, NULL);
    ts.tv_sec = tv.tv_sec + config_timeout_chunk;
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
        dtp_trace("timed out\n");
    }
    else if (rc == 0)
    {
        dtp_trace("Downloader thread for %s woken up! "
                  "Chunks remaining: %d\n",
                  direntry_get_base_name(thread->de),
                  thread->chunk_list_count);


        pthread_mutex_lock(&(thread->mutex));

        chunk = TAILQ_FIRST(&thread->chunk_list);
        /* there must be a chunk, because we just came out of the semaphore */
        assert(chunk);


        /* remove the chunk from the list */
        TAILQ_REMOVE(&thread->chunk_list, chunk, entries);
        dtp_trace("removed entry from chunk list. List now:");
        dump_chunk_list(thread);

        pthread_mutex_unlock(&(thread->mutex));
    }
    else
    {
        dtp_trace("pthread_cond_timedwait(): error\n");
        assert(0);
    }

    dtp_trace_dedent();


    return chunk;
}

/* Add a chunk to the chunk list for an individual thread.
 * These go in in order - start of file to end of file */
/* EXECUTES IN THREAD: read().
 * Accesses a thread - ?no mutex needed */
static void thread_chunk_list_add_chunk (thread_t *thread,
                                         off_t start,
                                         off_t end,
                                         char *buf,
                                         void *ctxt        )
{
    chunk_t *c = NULL, *curr;


    /* proto-chunks (start and end offsets) must come in here partially
     * sanitised. They are not allowed to be completely out of the range of the
     * file, but may overshoot the end */
    assert(start < direntry_get_size(thread->de));
    if (end > direntry_get_size(thread->de))
    {
        /* fix the chunk up so that it within the extents of the file. This way
         * when the consumer is passed the last buffer of the file this chunk
         * ends too and will be disposed of. This means any remaining chunks
         * actually need something meaningful done with them */
        end = direntry_get_size(thread->de);
    }

    c = chunk_new(start, end, buf, ctxt);


    pthread_mutex_lock(&(thread->mutex));

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
    dtp_trace("added entry to chunk list. List now:");
    dump_chunk_list(thread);

    /* increment the chunk list count */
    pthread_mutex_lock(&thread->chunk_list_mutex);
    thread->chunk_list_count++;
    pthread_mutex_unlock(&thread->chunk_list_mutex);
    pthread_cond_signal(&thread->chunk_list_cond);

    pthread_mutex_unlock(&(thread->mutex));
}

/* This is the entry point for new downloader threads. */
/* EXECUTES IN THREAD: downloader_thread_main().
 * Accesses a thread - ?no mutex needed */
static void *downloader_thread_main (void *arg)
{
    thread_t *thread = (thread_t *)arg;
    char range[1024];
    int rc;
    int first_time = 1;


    dtp_trace("downloader_thread_main(file==%s): New downloader thread!\n",
              direntry_get_base_name(thread->de) );
    dtp_trace_indent();

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

        if (thread->current_chunk->start)
        {
            sprintf(range, "%" PRIu64 "-", thread->current_chunk->start);
        }
        thread->download_offset = thread->current_chunk->start;
        dtp_trace("fetching range \"%s\"\n", range);

        /* begin the download */
        rc = fetcher_fetch_file(direntry_get_hash(thread->de),
                                (thread->current_chunk->start) ? range : NULL,
                                (curl_write_callback)&thread_pool_consumer,
                                (void *)thread                              );
    } while (rc == 0);

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
            signal_read_thread(thread->current_chunk->ctxt, rc);
            chunk_delete(thread->current_chunk);
            thread->current_chunk = NULL;

            chunk_list_empty(thread, rc);
        }
    }

    /* Delete the data structures */
#if FEATURE_PROGRESS_METER
    progress_delete(direntry_get_base_name(thread->de));
#endif
    thread_delete(thread); /* When we move to a pool, we'll need to blank it */

    dtp_trace("download_thread_main() returning\n");
    dtp_trace_dedent();


    pthread_exit(NULL);
}

static void chunk_list_empty (thread_t *thread, int rc)
{
    chunk_t *c;


    while ((c = TAILQ_FIRST(&thread->chunk_list)))
    {
        signal_read_thread(c->ctxt, rc);
        TAILQ_REMOVE(&thread->chunk_list, TAILQ_FIRST(&thread->chunk_list), entries);
        chunk_delete(c);
    }
}


/* EXECUTES IN THREAD: downloader_thread_main().
 * Accesses a thread - ?no mutex needed */
static size_t thread_pool_consumer (void *b, size_t size, size_t nmemb, void *userp)
{
    buf_t *buf = (buf_t *)malloc(sizeof(buf_t));
    thread_t *thread = (thread_t *)((fetcher_cb_data_t *)userp)->cb_data;
    chunk_t *chunk;
    size_t buf_len = size * nmemb, rc = -EIO, copy_len;


    dtp_trace("thread_pool_consumer(size==%zd, nmemb==%zd, userp==%p)\n",
              size, nmemb, userp);
    dtp_trace_indent();

    if (!thread->current_chunk) thread->current_chunk = chunk_get_next(thread);
    if (!thread->current_chunk)
    {
        rc = 0;
        thread->timed_out = 1;
        goto bail;
    }
    chunk = thread->current_chunk;

    buf->start = thread->download_offset;
    buf->end   = thread->download_offset + buf_len;
    buf->buf   = b;


    while (1)
    {
        dtp_trace("buf: %#x-%#x; chunk: %#x-%#x\n", buf->start, buf->end, chunk->start, chunk->end);

        if (chunk->start == buf->start)
        {
            dtp_trace("chunk ok - continuing stream\n");

            /* How much to copy? */
            copy_len = MIN((buf->end - buf->start), (chunk->end - chunk->start));
            dtp_trace("copy_len == %#x\n", copy_len);

            /* DO IT */
            dtp_trace("doing it: absolute start == %#x, absolute end == %#x\n",
                      buf->start, buf->start + copy_len);
            memcpy(chunk->buf, buf->buf, copy_len);

            chunk->bytes_used += copy_len;

            buf->start += copy_len;
            buf->buf   += copy_len;
            dtp_trace("buf->start now == %#x\n", buf->start);
            chunk->start += copy_len;
            chunk->buf   += copy_len;
            dtp_trace("chunk->start now == %#x\n", buf->start);

#if FEATURE_PROGRESS_METER
            progress_update(direntry_get_base_name(thread->de),
                            direntry_get_size(thread->de),
                            buf->end );
#endif


            if (buf->start == buf->end && chunk->start == chunk->end)
            {
                dtp_trace("End of buf && chunk\n");

                /* End of chunk stuff
                 * (but don't get a new one because end of buf might mean end
                 * of file) */
                signal_read_thread(chunk->ctxt, chunk->bytes_used);

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
                dtp_trace("End of buf\n");
                break;
            }

            if (chunk->start == chunk->end)
            {
                dtp_trace("End of chunk\n");

                /* we've reached the end of this chunk's buffer. signal its
                 * semaphore so that its read() thread can wake and return */
                signal_read_thread(chunk->ctxt, chunk->bytes_used);

                /* free the chunk */
                chunk_delete(chunk);

                thread->current_chunk = chunk_get_next(thread);
                if (!thread->current_chunk)
                {
                    rc = 0;
                    thread->timed_out = 1;
                    goto bail;
                }
                chunk = thread->current_chunk;
                assert(chunk); /* shouldn't happen - chunk_get_next() should block */
            }
        }
        else
        {
            dtp_trace("chunk not ok - bailing out to seek\n");


            rc = 0; /* haven't consumed the whole buf, so curl will bail out
                       with an error and we end up back in
                       downloader_thread_main() */
            thread->seek = 1;
            goto bail;
        }
    }
    dtp_trace("buf->start caught buf->end - finished buf processing loop\n");

    thread->download_offset = buf->end;
    dtp_trace("offset now == %#x\n\n", thread->download_offset);
    rc = buf_len;


bail:
    free(buf);

    dtp_trace_dedent();


    return rc;
}

static void signal_read_thread (void *ctxt, int rc)
{
    dtp_trace("signalling read() thead - %d bytes / error\n", rc);

    read_signal_chunk_done(rc, ctxt);
}

static void dump_chunk (chunk_t *chunk)
{
    dtp_trace_np("%#x -> %#x -> %#x", chunk->start, chunk->bytes_used, chunk->end);
}

static void dump_chunk_list (thread_t *thread)
{
    chunk_t *curr;
    unsigned i = 0;


    if (TAILQ_EMPTY(&thread->chunk_list))
    {
        dtp_trace_np("  <empty>\n");
    }
    else
    {
        TAILQ_FOREACH(curr, &thread->chunk_list, entries)
        {
            dtp_trace_np("  [%u] ", i++);
            dump_chunk(curr);
        }
        dtp_trace_np("\n");
    }
}
