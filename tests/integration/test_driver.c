/*
 * Integration tests "driver" - provides the main() symbol in integration test builds.
 *
 * Copyright (C) Matthew Turner 2010-2012. All rights reserved.
 *
 * $Id: test_driver.c 595 2012-03-27 13:53:09Z matt $
 */

#include <unistd.h>

#include "common.h"
#include "tests.h"


int main (int argc, char **argv)
{
    const char * test_dir;


    assert(argc == 2);

    test_dir = argv[1];
    assert(!access(test_dir, R_OK));


    list_test(test_dir);
    read_test(test_dir);


    return 0;
}
