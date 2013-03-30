/*
 * Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
 *
 * Fetcher API. For arranging the actual downloading of files.
 */

#ifndef _INCLUDED_FETCHER_H
#define _INCLUDED_FETCHER_H

#include "common.h"

#include <curl/curl.h>

#include "indexnodes.h"
#include "listing.h"


typedef struct
{
    CURL *eh;
    void *cb_data;
} fetcher_cb_data_t;


TRACE_DECLARE(fetcher)
#define fetcher_trace(...) TRACE(fetcher,__VA_ARGS__)
#define fetcher_trace_indent() TRACE_INDENT(fetcher)
#define fetcher_trace_dedent() TRACE_DEDENT(fetcher)


extern int fetcher_init (void);
extern void fetcher_finalise (void);

extern int fetcher_fetch_file (listing_t           *li,
                               const char * const   range,
                               curl_write_callback  cb,
                               void                *cb_data);

extern int fetcher_fetch_stats (indexnodes_t        *ins,
                                curl_write_callback  cb,
                                void                *cb_data);

extern int fetcher_fetch_internal (const char * const   url,
                                   const char * const   range,
                                   curl_write_callback  cb,
                                   void                *cb_data);

extern const char *fetcher_get_indexnode_protocol (proto_indexnode_t *in);
extern int http2errno (int http_code);

#endif /* _INCLUDED_FETCHER_H */
