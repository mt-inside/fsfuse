/*
 * Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
 *
 * Fetcher API. For arranging the actual downloading of files.
 */

#ifndef _INCLUDED_FETCHER_H
#define _INCLUDED_FETCHER_H

#include "common.h"


TRACE_DECLARE(fetcher)
#define fetcher_trace(...) TRACE(fetcher,__VA_ARGS__)
#define fetcher_trace_indent() TRACE_INDENT(fetcher)
#define fetcher_trace_dedent() TRACE_DEDENT(fetcher)


typedef struct _fetcher_t fetcher_t;

typedef int (*fetcher_header_cb_t)( void *ctxt, const char *key, const char *value );
typedef int (*fetcher_body_cb_t)(   void *ctxt, void *data, size_t len );


extern int fetcher_init (void);
extern void fetcher_finalise (void);

extern fetcher_t *fetcher_new (const char *url);
extern void fetcher_delete (fetcher_t *fetcher);

extern int fetcher_fetch_headers(
    fetcher_t *fetcher,
    fetcher_header_cb_t header_cb,
    void *header_cb_ctxt
);
extern int fetcher_fetch_body(
    fetcher_t *fetcher,
    fetcher_body_cb_t body_cb,
    void *body_cb_ctxt,
    const char *range
);

extern const char *fetcher_make_http_url (
    const char *host,
    const char *port,
    const char *path
);

extern const char *fetcher_escape_for_http ( const char *str );

#endif /* _INCLUDED_FETCHER_H */
