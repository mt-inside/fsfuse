/*
 * Unit tests "driver" - provides the main() symbol in unit test builds.
 *
 * Copyright (C) Matthew Turner 2010-2011. All rights reserved.
 *
 * $Id$
 */

#include "common.h"
#include "tests.h"


int main (int argc, char **argv)
{
    NOT_USED(argc);
    NOT_USED(argv);

    common_test();
    hash_test();
    http_test();
    string_buffer_test();


    return 0;
}
