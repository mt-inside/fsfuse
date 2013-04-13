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


TRACE_DECLARE(fetcher)
#define fetcher_trace(...) TRACE(fetcher,__VA_ARGS__)
#define fetcher_trace_indent() TRACE_INDENT(fetcher)
#define fetcher_trace_dedent() TRACE_DEDENT(fetcher)


extern int fetcher_init (void);
extern void fetcher_finalise (void);

extern int fetch(
    const char *url,
    curl_write_callback header_cb, void *header_cb_ctxt,
    curl_write_callback body_cb,   void *body_cb_ctxt,
    int headers_only,
    const char *range
);

extern const char *fetcher_make_http_url (
    const char *host,
    const char *port,
    const char *path
);

extern const char *fetcher_escape_for_http ( const char *str );

#endif /* _INCLUDED_FETCHER_H */
