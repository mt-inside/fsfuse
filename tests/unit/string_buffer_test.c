/*
 * String Buffer unit tests.
 *
 * Copyright (C) Matthew Turner 2008-2011. All rights reserved.
 *
 * $Id: hash_test.c 474 2010-01-29 00:01:07Z matt $
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
