/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Fetcher module. This contains the functions that arrange for actual file
 * transfers, be they of XML metadata files (e.g. dir listings), or actual data
 * (e.g. for read()).
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>

#include "common.h"
#include "config.h"
#include "curl_utils.h"
#include "fetcher.h"
#include "parser.h"
#include "indexnodes.h"
#include "peerstats.h"
#include "string_buffer.h"
#include "utils.h"


#define URL_LEN 1024


TRACE_DEFINE(fetcher)


static size_t indexnode_version_header_cb (void *ptr, size_t size, size_t nmemb, void *stream);

static size_t null_writefn (void *buf, size_t size, size_t nmemb, void *userp);


/* ========================================================================== */
/*      Init & Teardown                                                       */
/* ========================================================================== */

int fetcher_init (void)
{
    fetcher_trace("fetcher_init()\n");

    /* CURL_GLOBAL_SSL leaks memory, and isn't needed, yet... */
    curl_global_init(CURL_GLOBAL_NOTHING);


    return 0;
}

void fetcher_finalise (void)
{
    curl_global_cleanup();
}


/* ========================================================================== */
/*      File Operations                                                       */
/* ========================================================================== */

int fetcher_fetch_file (listing_t           *li,
                        const char * const   range,
                        curl_write_callback  cb,
                        void                *cb_data)
{
    int rc = EIO;
    char *url;
    listing_list_t *lis;


    assert(li);

    fetcher_trace("fetcher_fetch_file(hash: %s, range: \"%s\")\n", listing_get_hash(li), range);
    fetcher_trace_indent();


    /* Find alternatives */
    url = listing_make_url(li, "alternatives", listing_get_hash(li));
    rc = parser_fetch_listing(url, &lis);
    free(url);

    if (lis)
    {
        li = peerstats_chose_alternative(lis);

        /* Get the file */
        if (li)
        {
            rc = fetcher_fetch_internal(listing_get_href(li), range, cb, cb_data);
            listing_delete(CALLER_INFO li);
        }
        else
        {
            rc = EBUSY;
            /* TODO sort out handling of this whole area */
        }

        listing_list_delete(CALLER_INFO lis);
    }

    fetcher_trace_dedent();


    return rc;
}

int fetcher_fetch_stats (curl_write_callback  cb,
                         void                *cb_data)
{
    int rc;
    char *url;
    indexnodes_list_t *ins;
    indexnodes_list_item_t *item;


    fetcher_trace("fetcher_fetch_stats()\n");
    fetcher_trace_indent();

    ins = indexnodes_get();
    TAILQ_FOREACH(item, &ins->list, next)
    {
        url = indexnode_make_url(item->in, "stats", "");
        rc = fetcher_fetch_internal(url, NULL, cb, cb_data);
        free(url);
    }
    indexnodes_list_delete(ins);

    fetcher_trace_dedent();


    return rc;
}

int fetcher_fetch_internal (const char * const   url,
                            const char * const   range,
                            curl_write_callback  cb,
                            void                *cb_data)
{
    char *redirect_target, *error_buffer;
    string_buffer_t *alias = string_buffer_new();
    int rc = EIO, curl_rc;
    long http_code = 0;
    CURL *eh;
    struct curl_slist *slist = NULL;
    fetcher_cb_data_t cb_data_real;


    assert(url); assert(*url);

    fetcher_trace("fetcher_fetch_internal(url: %s, range: \"%s\")\n", url, range);
    fetcher_trace_indent();


    /* New handle */
    eh = curl_eh_new();

    /* Error buffer */
    error_buffer = (char *)malloc(CURL_ERROR_SIZE * sizeof(char));
    if (error_buffer)
    {
        curl_easy_setopt(eh, CURLOPT_ERRORBUFFER, error_buffer);
    }

    /* Stuff away download-specific data */
    cb_data_real.eh = eh;
    cb_data_real.cb_data = cb_data;

    /* URL */
    curl_easy_setopt(eh, CURLOPT_URL, url);
    fetcher_trace("fetching url \"%s\"\n", url);

    /* Range */
    if (range)
    {
        curl_easy_setopt(eh, CURLOPT_RANGE, range);
    }

    /* Other headers */
    curl_easy_setopt(eh, CURLOPT_USERAGENT, FSFUSE_NAME "-" FSFUSE_VERSION);

    string_buffer_set(alias, "fs2-alias: ");
    string_buffer_append(alias, config_alias);
    slist = curl_slist_append(slist, string_buffer_peek(alias));
    string_buffer_delete(alias);
    curl_easy_setopt(eh, CURLOPT_HTTPHEADER, slist);

    /* No keep-alive */
    curl_easy_setopt(eh, CURLOPT_FRESH_CONNECT, 1);
    curl_easy_setopt(eh, CURLOPT_FORBID_REUSE, 1);

    /* Data consumer */
    if (cb)
    {
        curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, cb);
        curl_easy_setopt(eh, CURLOPT_WRITEDATA, &cb_data_real);
    }
    else
    {
        curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, &null_writefn);
    }

    /* Have libcurl abort on error (http >= 400) */
    curl_easy_setopt(eh, CURLOPT_FAILONERROR, 1);

    /* Do it */
    curl_rc = curl_easy_perform(eh);

    fetcher_trace("curl returned %d, error: %s\n", curl_rc,
            (curl_rc == CURLE_OK) ? "n/a" : error_buffer);

    if (curl_rc == CURLE_OK ||
        curl_rc == CURLE_HTTP_RETURNED_ERROR)
    {
        curl_easy_getinfo(eh, CURLINFO_RESPONSE_CODE, &http_code);
        fetcher_trace("http code %d\n", http_code);
    }

    switch (curl_rc)
    {
        case CURLE_OK:
        case CURLE_HTTP_RETURNED_ERROR:
            rc = http2errno(http_code);
            break;

        case CURLE_WRITE_ERROR:
            /* assume benign atm - i.e. we caused it for a seek or timeout, so
             * the appropriate flag will be set */
            rc = 0;
            break;

        case CURLE_PARTIAL_FILE:
            /* i.e. server closed the connection half way through */
            rc = 0;
            break;

        case CURLE_COULDNT_CONNECT:
            /* we see this when an indexnode redirects us to a client that's
             * disappeared, but the indexnode hasn't noticed yet */
            rc = ENOENT;
            break;

        default:
            rc = EIO;
            break;
    }

    curl_easy_getinfo(eh, CURLINFO_EFFECTIVE_URL, &redirect_target);
    fetcher_trace("eventually redirected to: %s\n", redirect_target);


    curl_easy_setopt(eh, CURLOPT_RANGE, NULL);
    curl_eh_delete(eh);
    curl_slist_free_all(slist);
    free(error_buffer);


    fetcher_trace_dedent();


    return rc;
}

/* TODO: factor the first part of this into fetcher_setup_common() */
char *fetcher_get_indexnode_version (proto_indexnode_t *pin,
                                     indexnode_version_cb_t cb)
{
    char *url, *error_buffer, *version;
    string_buffer_t *alias = string_buffer_new();
    int curl_rc;
    long http_code;
    CURL *eh;
    struct curl_slist *slist = NULL;
    indexnode_version_cb_pair_t *pair =
        malloc(sizeof(indexnode_version_cb_pair_t));


    fetcher_trace("fetcher_get_indexnode_version(%p)\n", pin);
    fetcher_trace_indent();

    /* Package data for header callback */
    pair->indexnode = pin;
    pair->callback = cb;

    /* New handle */
    eh = curl_eh_new();

    /* Error buffer */
    error_buffer = (char *)malloc(CURL_ERROR_SIZE * sizeof(char));
    if (error_buffer)
    {
        curl_easy_setopt(eh, CURLOPT_ERRORBUFFER, error_buffer);
    }

    /* URL */
    url = proto_indexnode_make_url(pin, "browse", "");
    curl_easy_setopt(eh, CURLOPT_URL, url);

    /* Other headers */
    curl_easy_setopt(eh, CURLOPT_USERAGENT, FSFUSE_NAME "-" FSFUSE_VERSION);

    string_buffer_set(alias, "fs2-alias: ");
    string_buffer_append(alias, config_alias);
    slist = curl_slist_append(slist, string_buffer_peek(alias));
    string_buffer_delete(alias);
    curl_easy_setopt(eh, CURLOPT_HTTPHEADER, slist);

    /* No keep-alive */
    curl_easy_setopt(eh, CURLOPT_FRESH_CONNECT, 1);
    curl_easy_setopt(eh, CURLOPT_FORBID_REUSE, 1);

    /* Header consumer */
    curl_easy_setopt(eh, CURLOPT_HEADERFUNCTION, &indexnode_version_header_cb);
    curl_easy_setopt(eh, CURLOPT_HEADERDATA, pair);

    /* Do a HEAD request */
    curl_easy_setopt(eh, CURLOPT_NOBODY, 1);

    /* Do it - blocks */
    curl_rc = curl_easy_perform(eh);

    version = pair->version;
    free(pair);

    fetcher_trace("curl returned %d, error: %s\n", curl_rc,
        (curl_rc == CURLE_OK) ? "n/a" : error_buffer);

    if (curl_rc == CURLE_OK ||
        curl_rc == CURLE_HTTP_RETURNED_ERROR)
    {
        curl_easy_getinfo(eh, CURLINFO_RESPONSE_CODE, &http_code);
        fetcher_trace("http code %d\n", http_code);
    }

    fetcher_trace_dedent();

    return version;
}

static size_t indexnode_version_header_cb (void *ptr, size_t size, size_t nmemb, void *stream)
{
    indexnode_version_cb_pair_t *pair = (indexnode_version_cb_pair_t *)stream;
    char *header = (char *)ptr;


    NOT_USED(stream);

    if (!strncasecmp(header, "fs2-version: ", strlen("fs2-version: ")))
    {
        header += strlen("fs2-version: ");
        pair->version = pair->callback(pair->indexnode, header);
    }


    return size * nmemb;
}

/* We currently throw HTTP error codes around internally, because they're quite
 * nice and we only deal with HTTP atm. If, in future, we become transport
 * agnostic, we might change to some independent representation. As we're a
 * filesystem, this new representation would likely be similar to filesystem
 * error codes. */

/* Turn an http error code into a unix errno code.
 * See:
 * - RFC2616 (HTTP/1.1) http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
 * - errno.h
 * Most of these codes indicate that we've sent rubbish, or the server is
 * talking rubbish. In release builds we just return EIO, and in debug builds we
 * assert so the bug can be found */
int http2errno (int http_code)
{
    int rc = EIO;


    switch (http_code)
    {
        /* 1xx: Informational */
        case 100: /* Continue */
        case 101: /* Switching Protocols */
            break;

        /* 2xx: Successful */
        case 200: /* OK */
            rc = 0;
            break;
        case 201: /* Created */
        case 202: /* Accepted */
        case 203: /* Non-Authoritative Information */
        case 204: /* No Content */
        case 205: /* Reset Content */
            break;
        case 206: /* Partial Content */
            rc = 0;
            break;

        /* 3xx: Redirection */
        case 300: /* Multiple Choices */
        case 301: /* Moved Permanently */
        case 302: /* Found */
        case 303: /* See Other */
        case 304: /* Not Modified */
        case 305: /* Use Proxy */
        case 306: /* (Unused) */
        case 307: /* Temporary Redirect */
            break;

        /* 4xx: Client Error */
        /* i.e. some indicate user error, and some fsfuse bugs */
        case 400: /* Bad Request */
        case 401: /* Unauthorized */
        case 402: /* Payment Required */
        case 403: /* Forbidden */
            break;
        case 404: /* Not Found */
            rc = ENOENT;
            break;
        case 405: /* Method Not Allowed */
        case 406: /* Not Acceptable */
        case 407: /* Proxy Authentication Required */
        case 408: /* Request Timeout */
        case 409: /* Conflict */
        case 410: /* Gone */
        case 411: /* Length Required */
        case 412: /* Precondition Failed */
        case 413: /* Request Entity Too Large */
        case 414: /* Request-URI Too Long */
        case 415: /* Unsupported Media Type */
        case 416: /* Requested Range Not Satisfiable */
        case 417: /* Expectation Failed */
            break;

        /* 5xx: Server Error */
        case 500: /* Internal Server Error */
            rc = EIO;
            break;
        case 501: /* Not Implemented */
        case 502: /* Bad Gateway */
            break;
        case 503: /* Service Unavailable */
            /* Returned when sharer's upload limit has been reached */
            rc = EBUSY;
            break;
        case 504: /* Gateway Timeout */
        case 505: /* HTTP Version Not Supported */
            break;

        default: /* Not in the HTTP/1.1 spec */
            break;
    }

    if (rc == EIO)
    {
        trace_warn("http2errno: unknown HTTP return code %d\n", http_code);
    }


    return rc;
}


/* ========================================================================== */
/* Misc                                                                       */
/* ========================================================================== */

static size_t null_writefn (void *buf, size_t size, size_t nmemb, void *userp)
{
    NOT_USED(buf);
    NOT_USED(userp);

    return size * nmemb;
}
