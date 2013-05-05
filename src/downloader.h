/*
 * Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
 *
 * Download Thread "Class" - a thread for downloading a file.
 */

#ifndef _INCLUDED_DOWNLOADER_H
#define _INCLUDED_DOWNLOADER_H

#include <sys/types.h>

#include "common.h"
#include "direntry.h"


TRACE_DECLARE(downloader)
#define downloader_trace(...) TRACE(downloader,__VA_ARGS__)
#define downloader_trace_indent() TRACE_INDENT(downloader)
#define downloader_trace_dedent() TRACE_DEDENT(downloader)


typedef struct _downloader_t downloader_t;

typedef void (*chunk_done_cb_t)(void *ctxt, int rc, size_t size);


extern downloader_t *downloader_new (direntry_t *de);
extern void downloader_chunk_add (downloader_t *thread,
                                       off_t start,
                                       off_t end,
                                       char *buf,
                                       chunk_done_cb_t cb,
                                       void *ctxt          );

#endif /* _INCLUDED_DOWNLOADER_H */
