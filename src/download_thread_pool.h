/*
 * External API for the file downloader thread pool.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#ifndef _included_download_thread_pool_h
#define _included_download_thread_pool_h

#include <sys/types.h>

#include "common.h"
#include "direntry.h"


TRACE_DECLARE(dtp)


typedef void (*chunk_done_cb_t)(void *ctxt, int rc);


extern int thread_pool_init (void);
extern void thread_pool_finalise (void);
extern void thread_pool_chunk_add (direntry_t *de,
                                   off_t start,
                                   off_t end,
                                   char *buf,
                                   chunk_done_cb_t cb,
                                   void *ctxt          );

#endif /* _included_download_thread_pool_h */
