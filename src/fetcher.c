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

#include "common.h"

#include <assert.h>
#include <curl/curl.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fetcher.h"

#include "config_manager.h"
#include "config_reader.h"
#include "fs2_constants.h"
#include "indexnodes.h"
#include "indexnodes_list.h"
#include "indexnodes_iterator.h"
#include "string_buffer.h"
#include "utils.h"


#define URL_LEN 1024


TRACE_DEFINE(fetcher)


struct _fetcher_t
{
    const char *url;
    CURL *eh;
    char *error_buffer;
    struct curl_slist *slist;
};

typedef struct
{
    fetcher_header_cb_t cb;
    void *ctxt;
} header_cb_wrapper_ctxt_t;
typedef struct
{
    fetcher_body_cb_t cb;
    void *ctxt;
} body_cb_wrapper_ctxt_t;


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


fetcher_t *fetcher_new (const char *url)
{
    config_reader_t *config = config_get_reader();
    fetcher_t *fetcher = malloc(sizeof(*fetcher));
    string_buffer_t *alias = string_buffer_new();


    fetcher->url = url;

    /* New handle */
    fetcher->eh = curl_easy_init();

    /* Error buffer */
    fetcher->error_buffer = (char *)malloc(CURL_ERROR_SIZE * sizeof(char));
    curl_easy_setopt(fetcher->eh, CURLOPT_ERRORBUFFER, fetcher->error_buffer);

    /* URL */
    curl_easy_setopt(fetcher->eh, CURLOPT_URL, fetcher->url);

    /* Follow redirects */
    curl_easy_setopt(fetcher->eh, CURLOPT_FOLLOWLOCATION, 1);

    /* Other headers */
    curl_easy_setopt(fetcher->eh, CURLOPT_USERAGENT, FSFUSE_NAME "-" FSFUSE_VERSION);

    string_buffer_append(alias, strdup(fs2_alias_header_key));
    string_buffer_append(alias, config_alias(config));
    fetcher->slist = NULL;
    fetcher->slist = curl_slist_append(fetcher->slist, string_buffer_peek(alias));
    string_buffer_delete(alias);
    curl_easy_setopt(fetcher->eh, CURLOPT_HTTPHEADER, fetcher->slist);

    /* No keep-alive */
    curl_easy_setopt(fetcher->eh, CURLOPT_FRESH_CONNECT, 1);
    curl_easy_setopt(fetcher->eh, CURLOPT_FORBID_REUSE, 1);

    /* Have libcurl abort on error (http >= 400) */
    curl_easy_setopt(fetcher->eh, CURLOPT_FAILONERROR, 1);

    config_reader_delete(config);


    return fetcher;
}

void fetcher_delete (fetcher_t *fetcher)
{
    curl_easy_cleanup(fetcher->eh);
    curl_slist_free_all(fetcher->slist);
    free(fetcher->error_buffer);
    free_const(fetcher->url);

    free(fetcher);
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
static int http2errno (int http_code)
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

static int process_curl_response( fetcher_t *fetcher, int curl_rc )
{
    char *redirect_target;
    long http_code;
    int rc;


    switch (curl_rc)
    {
        case CURLE_OK:
        case CURLE_HTTP_RETURNED_ERROR:
            curl_easy_getinfo(fetcher->eh, CURLINFO_RESPONSE_CODE, &http_code);
            fetcher_trace("http code %d\n", http_code);
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

    fetcher_trace("curl returned %d, error: %s\n", curl_rc,
            (curl_rc == CURLE_OK) ? "n/a" : fetcher->error_buffer);

    curl_easy_getinfo(fetcher->eh, CURLINFO_EFFECTIVE_URL, &redirect_target);
    fetcher_trace("eventually redirected to: %s\n", redirect_target);


    return rc;
}

/* These header lines come in complete with their trailing new-lines.
 * HTTP/1.1 (RFC-2616) states that this sequence is \r\n
 *   (http://www.w3.org/Protocols/rfc2616/rfc2616-sec2.html#sec2.2)
 * From the libcurl API docs:
 *   "Do not assume that the header line is zero terminated!"
 */
static size_t header_cb_wrapper( char *header, size_t size, size_t nmemb, void *ctxt )
{
    header_cb_wrapper_ctxt_t *wrapper_ctxt = (header_cb_wrapper_ctxt_t *)ctxt;
    size_t len = size * nmemb, key_len, value_len;
    char *colon, *key, *value;
    size_t rc = len;

    colon = memchr( header, ':', len );
    if( colon )
    {
        key_len = colon - header;
        key = malloc( key_len + 1 );
        strncpy( key, header, key_len );
        key[key_len] = '\0';

        value_len = len - key_len - 2; /* -2 for line-end */
        value = malloc( value_len + 1 ); /* +1 for terminating \0 */
        strncpy( value, header + key_len, value_len );
        value[value_len] = '\0';

        rc = wrapper_ctxt->cb( wrapper_ctxt->ctxt, key, value ) ? 0 : len;
    }

    return rc;
}

static size_t body_cb_wrapper( char *data, size_t size, size_t nmemb, void *ctxt )
{
    body_cb_wrapper_ctxt_t *wrapper_ctxt = (body_cb_wrapper_ctxt_t *)ctxt;

    return wrapper_ctxt->cb( wrapper_ctxt->ctxt, data, size * nmemb ) ? 0 : size * nmemb;
}

int fetcher_fetch_headers(
    fetcher_t *fetcher,
    fetcher_header_cb_t header_cb,
    void *header_cb_ctxt
)
{
    header_cb_wrapper_ctxt_t *header_cb_wrapper_ctxt = NULL;
    int rc;


    assert(header_cb);

    /* Do a HEAD request */
    curl_easy_setopt(fetcher->eh, CURLOPT_NOBODY, 1);

    /* Header consumer */
    header_cb_wrapper_ctxt = malloc(sizeof(*header_cb_wrapper_ctxt));
    header_cb_wrapper_ctxt->cb = header_cb;
    header_cb_wrapper_ctxt->ctxt = header_cb_ctxt;

    curl_easy_setopt(fetcher->eh, CURLOPT_HEADERFUNCTION, &header_cb_wrapper);
    curl_easy_setopt(fetcher->eh, CURLOPT_HEADERDATA, header_cb_ctxt);


    /* Do it - blocks */
    rc = process_curl_response( fetcher, curl_easy_perform(fetcher->eh) );


    free( header_cb_wrapper_ctxt );


    return rc;
}

int fetcher_fetch_body(
    fetcher_t *fetcher,
    fetcher_body_cb_t body_cb,
    void *body_cb_ctxt,
    const char *range
)
{
    body_cb_wrapper_ctxt_t *body_cb_wrapper_ctxt = NULL;
    int rc;


    /* Body consumer */
    body_cb_wrapper_ctxt = malloc(sizeof(*body_cb_wrapper_ctxt));
    body_cb_wrapper_ctxt->cb = body_cb;
    body_cb_wrapper_ctxt->ctxt = body_cb_ctxt;

    curl_easy_setopt(fetcher->eh, CURLOPT_WRITEFUNCTION, &body_cb_wrapper);
    curl_easy_setopt(fetcher->eh, CURLOPT_WRITEDATA, body_cb_ctxt);

    /* Range */
    if (range)
    {
        curl_easy_setopt(fetcher->eh, CURLOPT_RANGE, range);
    }


    /* Do it - blocks */
    rc = process_curl_response( fetcher, curl_easy_perform( fetcher->eh ) );


    free( body_cb_wrapper_ctxt );
    curl_easy_setopt(fetcher->eh, CURLOPT_RANGE, NULL);


    return rc;
}


/* ========================================================================== */
/* URL Operations                                                             */
/* ========================================================================== */

/* Is this string IPv4 address? */
static int is_ip4_address (const char *s)
{
    return (strspn(s, "0123456789.") == strlen(s));
}

/* Is this string IPv6 address? */
static int is_ip6_address (const char *s)
{
    return (strspn(s, "0123456789abcdefABCDEF:") == strlen(s));
}


const char *fetcher_make_http_url (
    const char *host,
    const char *port,
    const char *path
)
{
    string_buffer_t *sb = string_buffer_new( );
    char *fmt, *url;


    assert(host); assert(*host);
    assert(port); assert(*port);
    assert(path);


    if( is_ip4_address( host ) || is_ip6_address( host ))
    {
        fmt = "http://[%s]:%s/%s";
    }
    else
    {
        fmt = "http://%s:%s/%s";
    }


    string_buffer_printf( sb, fmt, host, port, path );
    url = string_buffer_commit( sb );


    return url;
}

const char *fetcher_escape_for_http ( const char *str )
{
    CURL *eh = curl_easy_init( );
    const char *esc = curl_easy_escape( eh, str, 0 );


    free_const( str );
    curl_easy_cleanup( eh );


    return esc;
}
