/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Unit tests "driver" - provides the main() symbol in unit test builds.
 */

#include "common.h"
#include "tests.h"


int main (int argc, char **argv)
{
    NOT_USED(argc);
    NOT_USED(argv);

    hash_table_test();
    http_test();
    string_buffer_test();
    uri_test();
    utils_test();


    return 0;
}
