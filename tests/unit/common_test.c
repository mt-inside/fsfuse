/*
 * Common utility module unit tests
 *
 * Copyright (C) Matthew Turner 2008-2010. All rights reserved.
 *
 * $Id$
 */

#include <string.h>
#include <stdlib.h>

#include "common.h"
#include "tests.h"


static void test_basename_dirname (void)
{
    const char *a = "/foo/bar/baz",
               *b = "/foo",
               *c = "foo",
               *d = "/",
               *e = "";

    assert(!strcmp(fsfuse_basename(a), "baz"));
    assert(!strcmp(fsfuse_dirname(a), "/foo/bar"));

    assert(!strcmp(fsfuse_basename(b), "foo"));
    assert(!strcmp(fsfuse_dirname(b), "/"));

    assert(!strcmp(fsfuse_basename(c), "foo"));
    assert(!strcmp(fsfuse_dirname(c), ""));

    assert(!strcmp(fsfuse_basename(d), ""));
    assert(!strcmp(fsfuse_dirname(d), "/"));

    assert(!strcmp(fsfuse_basename(e), ""));
    assert(!strcmp(fsfuse_dirname(e), ""));
}

static void test_compare_dotted_version (void)
{
    assert(compare_dotted_version("1", "2") == -1);
    assert(compare_dotted_version("2", "1") == 1);
    assert(compare_dotted_version("1", "1") == 0);
    assert(compare_dotted_version("0", "0") == 0);

    assert(compare_dotted_version("0.1", "0.2") == -1);
    assert(compare_dotted_version("0.2", "0.1") == 1);
    assert(compare_dotted_version("0.1", "0.1") == 0);
    assert(compare_dotted_version("0.0", "0.0") == 0);

    assert(compare_dotted_version("0.5.1", "0.5.2") == -1);
    assert(compare_dotted_version("0.5.2", "0.5.1") == 1);
    assert(compare_dotted_version("0.5.1", "0.5.1") == 0);
    assert(compare_dotted_version("0.5.0", "0.5.0") == 0);

    assert(compare_dotted_version("0.2", "0.2.0") == 0);
    assert(compare_dotted_version("0.2", "0.2.1") == -1);
    assert(compare_dotted_version("0.2", "0.2.2") == -1);
    assert(compare_dotted_version("0.2", "0.2.3") == -1);
    assert(compare_dotted_version("0.2", "0.1.9") == 1);

    assert(compare_dotted_version("0.2.0", "0.2") == 0);
    assert(compare_dotted_version("0.2.1", "0.2") == 1);
    assert(compare_dotted_version("0.2.2", "0.2") == 1);
    assert(compare_dotted_version("0.2.3", "0.2") == 1);
    assert(compare_dotted_version("0.1.9", "0.2") == -1);

    assert(compare_dotted_version("0.2", "0.2.0.0.1") == -1);
    assert(compare_dotted_version("0.2.0.0.1", "0.2") == 1);

    assert(compare_dotted_version("1", "666") == -1);
    assert(compare_dotted_version("11111.11111.22", "11111.11111.22") == 0);

    assert(compare_dotted_version("0.10", "0.1") == 1);
    assert(compare_dotted_version("0.10", "0.1.0") == 1);
    assert(compare_dotted_version("0.10", "0.10") == 0);
    assert(compare_dotted_version("0.10", "0.10.0") == 0);
    assert(compare_dotted_version("0.10", "0.11") == -1);

    assert(compare_dotted_version("0.1", "0.10") == -1);
    assert(compare_dotted_version("0.1.0", "0.10") == -1);
    assert(compare_dotted_version("0.10", "0.10") == 0);
    assert(compare_dotted_version("0.10.0", "0.10") == 0);
    assert(compare_dotted_version("0.11", "0.10") == 1);
}

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

void common_test (void)
{
    test_basename_dirname();
    test_compare_dotted_version();
    test_uri();
}
