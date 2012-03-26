/*
 * HTTP module implementation.
 * A simple module for sending simple HTTP requests and receiving simple
 * replies.
 *
 * Copyright (C) Matthew Turner 2010. All rights reserved.
 *
 * $Id: download_thread_pool.c 513 2010-03-08 22:27:27Z matt $
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

#include "http.h"
#include "string_buffer.h"
#include "uri.h"


TRACE_DEFINE(http)


typedef enum
{
    http_req_state_NEW,
    http_req_state_CONNECTING,
    http_req_state_CONNECTED,
    http_req_state_SENDING,
    http_req_state_WAITING_HEADERS,
    http_req_state_WAITING_DATA,
    http_req_state_DONE
} http_req_state_t;

struct _http_req_t
{
    uri_t *uri;
    http_req_state_t state;

    const char **headers;
    unsigned header_count;

    http_header_cb_t header_cb;
    void *           header_cb_ctxt;
    http_data_cb_t   data_cb;
    void *           data_cb_ctxt;
};


static const char **header_find (const char **headers,
                                 unsigned count,
                                 const char *key)
{
    unsigned i;
    const char *header = NULL;


    for (i = 0; i < count; ++i)
    {
        header = *(headers + (i * 2));
        if (!strcmp(header, key)) break;
    }


    return i < count ? headers + (i * 2) : NULL;
}

static const char *make_req_string (http_req_t *req)
{
    unsigned count, i;
    const char **headers, *key, *val;
    string_buffer_t *buf;
    char *ret = NULL, *path;


    assert(req);

    buf = string_buffer_new();
    if (buf)
    {
        path = uri_get_path(req->uri);
        count = req->header_count;
        headers = req->headers;

        assert(path);

        string_buffer_append(buf, "GET ");
        string_buffer_append(buf, path);
        string_buffer_append(buf, " HTTP/1.1\n\n");

        free(path);

        for (i = 0; i < count; ++i)
        {
            key = *(headers + (i * 2));
            val = *(headers + (i * 2) + 1);

            string_buffer_append(buf, key);
            string_buffer_append(buf, ": ");
            string_buffer_append(buf, val);
            string_buffer_append(buf, "\n");
        }

        ret = string_buffer_commit(buf);
    }


    return ret;
}

static int establish_connection (http_req_t *req)
{
    int s, rc;
    char *host, *port;
    struct addrinfo ai_hint, *ai_results, *ai;
    //FIXME: error handling


    assert(req);

    req->state = http_req_state_CONNECTING;

    memset(&ai_hint, 0, sizeof(struct addrinfo));
    ai_hint.ai_family = AF_UNSPEC;
    ai_hint.ai_socktype = SOCK_STREAM;

    host = uri_get_host(req->uri);
    port = uri_get_port(req->uri);

    rc = getaddrinfo(host, port, &ai_hint, &ai_results);
    if (rc)
    {
        //FIXME error
        gai_strerror(rc);
    }

    for (ai = ai_results; ai; ai = ai->ai_next)
    {
        assert(ai->ai_family == AF_INET || ai->ai_family == AF_INET6);
        assert(ai->ai_socktype == SOCK_STREAM);

        s = socket(ai->ai_family,
                   ai->ai_socktype,
                   ai->ai_protocol);
        if (s == -1) continue;

        if (connect(s, ai->ai_addr, ai->ai_addrlen) != -1) break;

        close(s);
    }
    if (!ai)
    {
        //FIXME: couldn't connect in any way
    }
    else
    {
        req->state = http_req_state_CONNECTED;
    }

    freeaddrinfo(ai_results);
    free(port);
    free(host);


    return s;
}

static void close_connection (int sock)
{
    close(sock);
}

static void sock_send_buf (int s, const char *buf, size_t len)
{
    int written;


    assert(s >= 0);
    assert(buf);

    while (len)
    {
        errno = 0;
        written = write(s, buf, len);

        if (written == -1)
        {
            if (errno == EINTR) continue;
            assert(0);
        }

        assert((size_t)written <= len);
        len -= written;
        buf += written;
    }
}

#define BUF_SIZE 1024
void sock_slurp (http_req_t *req, int sock)
{
    size_t red, buf_valid, buf_remain;
    char *buf_start, *buf_cur;


    assert(req);
    assert(sock != -1);

    req->state = http_req_state_WAITING_HEADERS;


    req->state = http_req_state_WAITING_DATA;

    do
    {
        buf_start = buf_cur = malloc(BUF_SIZE);
        buf_remain = BUF_SIZE;
        buf_valid = 0;

        do
        {
            red = read(sock, buf_cur, buf_remain);
            buf_cur += red;
            buf_valid += red;
            buf_remain -= red;
        } while (red && buf_remain);

        req->data_cb(req, req->data_cb_ctxt, buf_start, buf_valid);

    } while (red);

    req->state = http_req_state_DONE;
}


/* ========================================================================== */

int http_init (void)
{
    /* nothing to do */

    return 0;
}

void http_finalise (void)
{
}


http_req_t *http_req_new (void)
{
    http_req_t *req;


    req = calloc(sizeof(http_req_t), 1);

    if (req)
    {
        assert(req->state == http_req_state_NEW);
    }


    return req;
}

void http_req_delete (http_req_t *req)
{
    uri_delete(req->uri);

    free(req);
}


void http_req_set_uri (http_req_t *req, const char *uri_str)
{
    assert(req);
    assert(uri_str);

    req->uri = uri_from_string(uri_str);
}

void http_req_set_components (http_req_t *req,
                              const char *host,
                              const char *port,
                              const char *path)
{
    assert(req);
    assert(host);
    assert(port);
    assert(path);

    req->uri = uri_new();

    if (req->uri)
    {
        uri_set_scheme(req->uri, "http");
        uri_set_host(req->uri, host);
        uri_set_port(req->uri, port);
        uri_set_path(req->uri, path);
    }
}

void http_req_set_header (http_req_t *req,
                          const char *key,
                          const char *value)
{
    const char **header = header_find(req->headers, req->header_count, key);


    if (header)
    {
        *(header + 1) = strdup(value);
    }
    else
    {
        req->headers = realloc(req->headers,
            ((req->header_count + 1) * 2) * sizeof(char *));

        header = req->headers + req->header_count * 2;
        *header = strdup(key);
        *(header + 1) = strdup(value);

        req->header_count++;
    }
}

void http_req_set_header_cb (http_req_t *req,
                             http_header_cb_t cb,
                             void *ctxt)
{
    assert(req);

    req->header_cb = cb;
    req->header_cb_ctxt = ctxt;
}

void http_req_set_data_cb (http_req_t *req,
                           http_data_cb_t cb,
                           void *ctxt)
{
    assert(req);

    req->data_cb = cb;
    req->data_cb_ctxt = ctxt;
}

int http_req_send (http_req_t *req)
{
    int sock;
    const char *req_str;


    assert(req);

    sock = establish_connection(req);

    req_str = make_req_string(req);
    req->state = http_req_state_SENDING;
    sock_send_buf(sock, req_str, strlen(req_str));
    free_const(req_str);

    sock_slurp(req, sock);

    close_connection(sock);

    return 0; //TODO
}

/* ========================================================================== */

void http_req_dump_headers (http_req_t *req)
{
    unsigned i, count;
    const char **headers, *key, *val;


    assert(req);

    headers = req->headers;
    count = req->header_count;

    for (i = 0; i < count; ++i)
    {
        key = *(headers + (i * 2));
        val = *(headers + (i * 2) + 1);
        printf("header %u | %s: %s\n", i, key, val);
    }
}
