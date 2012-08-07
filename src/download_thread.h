/*
 * Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
 *
 * Download Thread "Class" - a thread for downloading a file.
 */

#ifndef _included_download_thread_h
#define _included_download_thread_h

#include <sys/types.h>

#include "common.h"
#include "direntry.h"
#include "download_thread_pool.h"


TRACE_DECLARE(dl_thr)
#define dl_thr_trace(...) TRACE(dl_thr,__VA_ARGS__)
#define dl_thr_trace_indent() TRACE_INDENT(dl_thr)
#define dl_thr_trace_dedent() TRACE_DEDENT(dl_thr)


typedef struct _thread_t thread_t;

typedef void (*thread_end_cb_t)(thread_t *thread);


extern thread_t *download_thread_new (direntry_t *de, thread_end_cb_t end_cb);
extern int download_thread_is_for (thread_t *thread, direntry_t *de);
extern void download_thread_chunk_add (thread_t *thread,
                                       off_t start,
                                       off_t end,
                                       char *buf,
                                       chunk_done_cb_t cb,
                                       void *ctxt          );

#endif /* _included_download_thread_h */
