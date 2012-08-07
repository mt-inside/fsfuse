/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * String Buffer unit tests.
 */

#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "tests.h"
#include "string_buffer.h"


void string_buffer_test (void)
{
    string_buffer_t *sb;
    char *s;


    sb = string_buffer_new();
    string_buffer_set(sb, "Hello");

    s = string_buffer_get(sb);
    assert(!strcmp(s, "Hello"));
    free(s);

    string_buffer_append(sb, ", World!");

    s = string_buffer_get(sb);
    assert(!strcmp(s, "Hello, World!"));
    free(s);

    string_buffer_delete(sb);


    sb = string_buffer_new();
    string_buffer_set(sb, "Hello");

    assert(!strcmp(string_buffer_peek(sb), "Hello"));

    string_buffer_append(sb, ", World!");

    assert(!strcmp(string_buffer_peek(sb), "Hello, World!"));

    string_buffer_delete(sb);


    sb = string_buffer_from_chars("Hello");

    s = string_buffer_get(sb);
    assert(!strcmp(s, "Hello"));
    free(s);

    string_buffer_append(sb, ", World!");

    s = string_buffer_get(sb);
    assert(!strcmp(s, "Hello, World!"));
    free(s);

    string_buffer_delete(sb);


    sb = string_buffer_from_chars("Hello");

    s = string_buffer_get(sb);
    assert(!strcmp(s, "Hello"));
    free(s);

    string_buffer_append(sb, ", World!");
    string_buffer_append(sb, " World!");
    string_buffer_append(sb, " World!");
    string_buffer_append(sb, " World!");
    string_buffer_append(sb, " World!");

    s = string_buffer_get(sb);
    assert(!strcmp(s, "Hello, World! World! World! World! World!"));
    free(s);

    string_buffer_delete(sb);
}
