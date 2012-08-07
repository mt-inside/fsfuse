/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * URI class unit tests
 */

#include <string.h>
#include <stdlib.h>

#include "common.h"
#include "tests.h"
#include "uri.h"


static void test_uri (void)
{
    uri_t *uri;
    const char *str;


    //URIs constructed from URI strings are not yet supported
    //uri = uri_from_string("http://user@foo.com/path/bar.html");


    uri = uri_new();

    uri_set_scheme(uri, "http");
    uri_set_userinfo(uri, "user");
    uri_set_host(uri, "foo.com");
    uri_set_port(uri, "1337");
    uri_set_path(uri, "/path/bar.html");
    uri_set_query(uri, "query");
    uri_set_fragment(uri, "frag");

    str = uri_get(uri);
    assert(!strcmp(str, "http://user@foo.com:1337/path/bar.html?query#frag"));
    free_const(str);

    uri_delete(uri);


    uri = uri_new();

    uri_set_scheme(uri, "http");
    uri_set_host(uri, "foo.com");
    uri_set_path(uri, "/path/bar.html");

    str = uri_get(uri);
    assert(!strcmp(str, "http://foo.com/path/bar.html"));
    free_const(str);

    uri_delete(uri);
}

void uri_test (void)
{
    test_uri();
}
