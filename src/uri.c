/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Uniform Resource Identifier Class
 */

#include "common.h"

#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "uri.h"
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
