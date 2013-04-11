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

extern int fetcher_get_indexnode_info (const char *indexnode_url,
                                       const char **protocol,
                                       const char **id);

/* TODO: why extrernal? */
extern int fetcher_fetch_internal (const char * const   url,
                                   const char * const   range,
                                   curl_write_callback  cb,
                                   void                *cb_data);

/* TODO: why public? */
extern int http2errno (int http_code);

extern const char *fetcher_make_http_url (
    const char *host,
    const char *port,
    const char *path
);

extern const char *fetcher_escape_for_http ( const char *str );

#endif /* _INCLUDED_FETCHER_H */
