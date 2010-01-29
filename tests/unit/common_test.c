/*
 * Common utility module unit tests
 *
 * Copyright (C) Matthew Turner 2008-2010. All rights reserved.
 *
 * $Id$
 */

#include <string.h>

#include "common.h"
#include "tests.h"


void common_test (void)
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
