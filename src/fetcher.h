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


typedef enum
{
    fetcher_url_type_t_PLAIN, /* no prefix */
    fetcher_url_type_t_BROWSE,
    fetcher_url_type_t_DOWNLOAD
} fetcher_url_type_t;

typedef struct
{
    CURL *eh;
    void *cb_data;
} fetcher_cb_data_t;


TRACE_DECLARE(fetcher)


extern int fetcher_init (void);
extern void fetcher_finalise (void);

extern int fetcher_fetch (const char * const   path,
                          fetcher_url_type_t   url_type,
                           const char * const   range,
                          curl_write_callback  cb,
                          void                *cb_data);
extern double fetcher_get_indexnode_version (void);
extern int http2errno (int http_code);

#endif /* _INCLUDED_FETCHER_H */
