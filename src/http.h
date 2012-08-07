/*
 * Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
 *
 * External API for the HTTP module.
 */

#ifndef _included_http_h
#define _included_http_h

#include "common.h"


TRACE_DECLARE(http)
#define http_trace(...) TRACE(http,__VA_ARGS__)
#define http_trace_indent() TRACE_INDENT(http)
#define http_trace_dedent() TRACE_DEDENT(http)


typedef struct _http_req_t http_req_t;

typedef int (*http_header_cb_t)(http_req_t *req,
                                void *ctxt,
                                const char *key,
                                const char *value);
typedef size_t (*http_data_cb_t)(http_req_t *req,
                                 void *ctxt,
                                 void *data,
                                 size_t len);


extern int http_init (void);
extern void http_finalise (void);


extern http_req_t *http_req_new (void);
extern void http_req_delete (http_req_t *req);

extern void http_req_set_uri (http_req_t *req, const char *uri_str);
extern void http_req_set_components (http_req_t *req,
                                     const char *host,
                                     const char *port,
                                     const char *path);

extern void http_req_set_header (http_req_t *req,
                                 const char *key,
                                 const char *value);

extern void http_req_set_header_cb (http_req_t *req,
                                    http_header_cb_t cb,
                                    void *ctxt);
extern void http_req_set_data_cb (http_req_t *req,
                                  http_data_cb_t cb,
                                  void *ctxt);

extern int http_req_send (http_req_t *req);

#endif /* _included_http_h */
