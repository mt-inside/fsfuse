/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Integration tests "driver" - provides the main() symbol in integration test builds.
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
