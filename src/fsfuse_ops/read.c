/*
 * read() implementation.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#include <string.h>
#include <errno.h>
#include <fuse.h>
#include <curl/curl.h>
#include <stdlib.h>

#include "common.h"
#include "locks.h"
#include "read.h"
#include "direntry.h"
#include "fetcher.h"
#include "indexnode.h"
#include "download_thread_pool.h"


TRACE_DEFINE(read)


typedef struct
{
    pthread_mutex_t filled_mutex;
    int rc;
} read_context_t;


/* MUST return as many bytes as were asked for, unless error or EOF. */
int fsfuse_read (const char *path,
                 char *buf,
                 size_t size,
                 off_t offset,
                 struct fuse_file_info *fi)
{
    int rc; /* +ve: number of bytes read
               -ve: -ERRNO */
    direntry_t *de;


    NOT_USED(fi);

    read_trace("fsfuse_read(path==%s, size==%zd, offset==%ju)\n", path, size, offset);
    trace_indent();


    rc = direntry_get(path, &de);

    if (rc)
    {
        assert(!de);
    }
    else
    {
        assert(de);
    }

    if (!rc)
    {
        if (de->type == direntry_type_DIRECTORY)
        {
            rc = -EISDIR;
        }
        else
        {
            /* check file limits - despite always calling getattr() first (so it
             * knows the length), fuse sometimes asks for ranges completely
             * past the end of the file */
            if (offset >= de->size)
            {
                rc = 0;
            }
            else
            {
                read_context_t *read_ctxt = (read_context_t *)calloc(sizeof(read_context_t), 1);
                pthread_mutex_init(&read_ctxt->filled_mutex, NULL);
                pthread_mutex_lock(&read_ctxt->filled_mutex);


                thread_pool_chunk_add(de, offset, offset + size, buf, (void *)read_ctxt);

                /* wait until the downloader thread pool has filled buf */
                read_trace("fsfuse_read() waiting on full buffer...\n");
                pthread_mutex_lock(&read_ctxt->filled_mutex);
                read_trace("fsfuse_read() woken by full buffer semaphore - returning (chunk->end == %u)\n", offset + size);

                if (read_ctxt->rc != (signed)size)
                {
                    read_trace("bytes read != file size => EOF / error\n");
                }
                rc = read_ctxt->rc;


                pthread_mutex_destroy(&read_ctxt->filled_mutex);
                free(read_ctxt);
            }
        }
        direntry_delete(de);
    }

    trace_dedent();


    return rc;
}

void read_signal_chunk_done (int rc, void *ctxt)
{
    read_context_t *read_ctxt = (read_context_t *)ctxt;


    read_ctxt->rc = rc;
    pthread_mutex_unlock(&read_ctxt->filled_mutex);
}
