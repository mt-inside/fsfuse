/*
 * Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
 *
 * Fetcher API. For arranging the actual downloading of files.
 */

#ifndef _INCLUDED_FETCHER_H
#define _INCLUDED_FETCHER_H

#include <curl/curl.h>

#include "common.h"
#include "indexnode.h"


typedef void (*indexnode_version_cb_t)(indexnode_t *, char *);

typedef struct
{
    CURL *eh;
    void *cb_data;
} fetcher_cb_data_t;

typedef struct
{
    indexnode_t *indexnode;
    indexnode_version_cb_t callback;
} indexnode_version_cb_pair_t;


TRACE_DECLARE(fetcher)
#define fetcher_trace(...) TRACE(fetcher,__VA_ARGS__)
#define fetcher_trace_indent() TRACE_INDENT(fetcher)
#define fetcher_trace_dedent() TRACE_DEDENT(fetcher)


extern int fetcher_init (void);
extern void fetcher_finalise (void);

extern int fetcher_fetch_file (const char * const   path,
                               const char * const   range,
                               curl_write_callback  cb,
                               void                *cb_data);

extern int fetcher_fetch_stats (curl_write_callback  cb,
                                void                *cb_data);

extern int fetcher_fetch_internal (const char * const   url,
                                   const char * const   range,
                                   curl_write_callback  cb,
                                   void                *cb_data);

extern void fetcher_get_indexnode_version (indexnode_t *in,
                                           indexnode_version_cb_t cb);
extern int http2errno (int http_code);

#endif /* _INCLUDED_FETCHER_H */
