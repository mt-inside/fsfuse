/*
 * Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
 *
 * Helper methods for the CURL library.
 */

#ifndef _INCLUDED_CURL_UTILS_H
#define _INCLUDED_CURL_UTILS_H

#include "common.h"

#include <curl/curl.h>


extern CURL *curl_eh_new (void);
extern void curl_eh_delete (CURL *eh);

#endif /* _INCLUDED_CURL_UTILS_H */
