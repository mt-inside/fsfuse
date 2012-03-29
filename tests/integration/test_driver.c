/*
 * Integration tests "driver" - provides the main() symbol in integration test builds.
 *
 * Copyright (C) Matthew Turner 2010-2012. All rights reserved.
 *
 * $Id: test_driver.c 595 2012-03-27 13:53:09Z matt $
 */

#include "common.h"
#include "tests.h"


int main (int argc, char **argv)
{
    NOT_USED(argc);
    NOT_USED(argv);

    list_test();
    read_test();


    return 0;
}
