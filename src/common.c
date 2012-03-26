/*
 * Useful common utility functions.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "common.h"
#include "string_buffer.h"


struct _uri_t
{
    char *scheme;
    char *userinfo;
    char *host;
    char *port;
    char *path;
    char *query;
    char *fragment;
};


static pthread_key_t thread_index_key;
static unsigned next_thread_index = 1;


static void thread_index_destroy (void *i);


int common_init (void)
{
    int rc;


    rc = pthread_key_create(&thread_index_key, &thread_index_destroy);


    return rc;
}

void common_finalise (void)
{
    unsigned *i = (unsigned *)pthread_getspecific(thread_index_key);
    thread_index_destroy((void *)i);

    pthread_key_delete(thread_index_key);
}


/* There are versions of these functions glibc, but they're not thread-safe.
 * These implementations are not complete, e.g. they don't drop trailing '/'s,
 * but this isn't needed as we always deal in normalised paths.
 * These functions are called often, so should aim to be fast. */
char *fsfuse_dirname (const char *path)
{
    char *loc = strrchr(path, '/');
    unsigned dir_len;
    char *dir = NULL;


    if (!loc) return strdup("");

    dir_len = loc - path;
    if (!dir_len) return strdup("/");

    dir = (char *)malloc((dir_len + 1) * sizeof(char));
    if (dir)
    {
        memcpy(dir, path, dir_len);
        dir[dir_len] = '\0';
    }


    return dir;
}

#ifndef _GNU_SOURCE
#define memrchr(s,c,n) strrchr(s,c)
#endif
char *fsfuse_basename (const char *path)
{
    size_t path_len = strlen(path);
    char *loc = memrchr(path, '/', path_len);
    unsigned base_len;
    char *base = NULL;


    if (!loc) return strdup(path);

    base_len = path_len - (loc - path) + 1;
    if (!base_len) return strdup("");

    base = (char *)malloc((base_len + 1) * sizeof(char));
    if (base)
    {
        memcpy(base, loc + 1, base_len);
        base[base_len] = '\0';
    }


    return base;
}


unsigned fsfuse_get_thread_index (void)
{
    unsigned *i = (unsigned *)pthread_getspecific(thread_index_key);


    if (!i)
    {
        i = (unsigned *)malloc(sizeof(unsigned));
        *i = next_thread_index++;
        pthread_setspecific(thread_index_key, (void *)i);
    }


    return *i;
}

static void thread_index_destroy (void *i)
{
    free((unsigned *)i);
}

/* assumes both strings are well-formed */
int compare_dotted_version (const char *ver, const char *cmp)
{
    int n1, n2;

    while (1)
    {
        n1 = strtol(ver, (char **)&ver, 0);
        n2 = strtol(cmp, (char **)&cmp, 0);

        if (n1 < n2) return -1;
        if (n1 > n2) return 1;

        if (*ver == '\0' && *cmp == '\0') return 0;

        if (*ver) ver++;
        if (*cmp) cmp++;
    }
}

/* Is this string IPv4 address? */
int is_ip4_address (const char *s)
{
    return (strspn(s, "0123456789.") == strlen(s));
}

/* Is this string IPv6 address? */
int is_ip6_address (const char *s)
{
    return (strspn(s, "0123456789abcdefABCDEF:") == strlen(s));
}


uri_t *uri_new (void)
{
    uri_t *uri = calloc(sizeof(uri_t), 1);


    return uri;
}

uri_t *uri_from_string (const char *str)
{
    uri_t *uri = uri_new();


    assert(str);

    //FIXME: parse...
    assert(0); //NOT SUPPORTED YET


    return uri;
}

void uri_delete (uri_t *uri)
{
    assert(uri);

    if (uri->scheme)   free(uri->scheme);
    if (uri->userinfo) free(uri->userinfo);
    if (uri->host)     free(uri->host);
    if (uri->port)     free(uri->port);
    if (uri->path)     free(uri->path);
    if (uri->query)    free(uri->query);
    if (uri->fragment) free(uri->fragment);

    free(uri);
}

void uri_set_scheme (uri_t *uri, const char *scheme)
{
    assert(uri);

    if (uri->scheme) free(uri->scheme);
    uri->scheme = strdup(scheme);
}
void uri_set_userinfo (uri_t *uri, const char *userinfo)
{
    assert(uri);

    if (uri->userinfo) free(uri->userinfo);
    uri->userinfo = strdup(userinfo);
}
void uri_set_host (uri_t *uri, const char *host)
{
    assert(uri);

    if (uri->host) free(uri->host);
    uri->host = strdup(host);
}
void uri_set_port (uri_t *uri, const char *port)
{
    assert(uri);

    if (uri->port) free(uri->port);
    uri->port = strdup(port);
}
void uri_set_path (uri_t *uri, const char *path)
{
    assert(uri);

    if (uri->path) free(uri->path);
    uri->path = strdup(path);
}
void uri_set_query (uri_t *uri, const char *query)
{
    assert(uri);

    if (uri->query) free(uri->query);
    uri->query = strdup(query);
}
void uri_set_fragment (uri_t *uri, const char *fragment)
{
    assert(uri);

    if (uri->fragment) free(uri->fragment);
    uri->fragment = strdup(fragment);
}

char *uri_get (uri_t *uri)
{
    /* This could prove to be a bit slow - be careful how often you call it, or
     * optimise it :P */
    string_buffer_t *buf = string_buffer_new();
    char *auth;


    assert(uri);

    if (uri->scheme)
    {
        string_buffer_append(buf, uri->scheme);
        string_buffer_append(buf, ":");
    }

    auth = uri_get_authority(uri);
    if (auth)
    {
        string_buffer_append(buf, "//");
        string_buffer_append(buf, auth);
        free(auth);
    }

    assert(uri->path);
    string_buffer_append(buf, uri->path);

    if (uri->query)
    {
        string_buffer_append(buf, "?");
        string_buffer_append(buf, uri->query);
    }
    if (uri->fragment)
    {
        string_buffer_append(buf, "#");
        string_buffer_append(buf, uri->fragment);
    }


    return string_buffer_commit(buf);
}

char *uri_get_scheme (uri_t *uri)
{
    assert(uri);
    return strdup(uri->scheme);
}
char *uri_get_authority (uri_t *uri)
{
    string_buffer_t *buf = string_buffer_new();


    assert(uri);

    if (uri->userinfo)
    {
        string_buffer_append(buf, uri->userinfo);
        string_buffer_append(buf, "@");
    }

    if (uri->host)
    {
        string_buffer_append(buf, uri->host);
    }

    if (uri->port)
    {
        string_buffer_append(buf, ":");
        string_buffer_append(buf, uri->port);
    }


    return string_buffer_commit(buf);
}
char *uri_get_userinfo (uri_t *uri)
{
    assert(uri);
    return strdup(uri->userinfo);
}
char *uri_get_host (uri_t *uri)
{
    assert(uri);
    return strdup(uri->host);
}
char *uri_get_port (uri_t *uri)
{
    assert(uri);
    return strdup(uri->port);
}
char *uri_get_path (uri_t *uri)
{
    assert(uri);
    return strdup(uri->path);
}
char *uri_get_query (uri_t *uri)
{
    assert(uri);
    return strdup(uri->query);
}
char *uri_get_fragment (uri_t *uri)
{
    assert(uri);
    return strdup(uri->fragment);
}

int hash_equal (char *h1, char *h2)
{
    return !strcmp(h1, h2);
}
