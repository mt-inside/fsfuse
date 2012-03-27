/*
 * Common utility module unit tests
 *
 * Copyright (C) Matthew Turner 2008-2012. All rights reserved.
 *
 * $Id$
 */

#include <string.h>
#include <stdlib.h>

#include "common.h"
#include "tests.h"
#include "utils.h"


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


void utils_test (void)
{
    test_basename_dirname();
    test_compare_dotted_version();
}
