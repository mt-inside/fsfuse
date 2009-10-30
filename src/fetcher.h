/*
 * Fetcher API. For arranging the actual downloading of files.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#ifndef _INCLUDED_FETCHER_H
#define _INCLUDED_FETCHER_H

#include <curl/curl.h>

#include "common.h"


typedef struct
{
    CURL *eh;
    void *cb_data;
} fetcher_cb_data_t;


TRACE_DECLARE(fetcher)


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

extern double fetcher_get_indexnode_version (void);
extern int http2errno (int http_code);

extern char *make_url (
    const char * const path_prefix,
    const char * const resource
);
extern char *make_escaped_url (
    const char * const path_prefix,
    const char * const resource
);


#endif /* _INCLUDED_FETCHER_H */
