/*
 * Unit tests "driver" - provides the main() symbol in unit test builds.
 *
 * Copyright (C) Matthew Turner 2010. All rights reserved.
 *
 * $Id: alarms.c 407 2009-11-24 10:36:55Z matt $
 */

#include "common.h"


int main (int argc, char **argv)
{
    NOT_USED(argc);
    NOT_USED(argv);

    hash_test();
    common_test();


    return 0;
}
