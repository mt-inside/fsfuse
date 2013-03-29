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

#include "common.h"

#include <stdlib.h>
#include <string.h>

#include <check.h>
#include "tests.h"

#include "string_buffer.h"


static string_buffer_t *sb;

static void setup( void )
{
    sb = string_buffer_new( );
}

static void teardown( void )
{
}


START_TEST( can_set_and_commit )
{
    char *s;

    string_buffer_set(sb, strdup( "Hello" ));

    s = string_buffer_commit(sb);
    ck_assert_str_eq( s, "Hello" );
    free(s);
}
END_TEST

START_TEST( can_set_append_and_commit )
{
    char *s;

    string_buffer_set(sb, strdup( "Hello" ));
    string_buffer_append(sb, strdup( ", World!" ));

    s = string_buffer_commit(sb);
    ck_assert_str_eq( s, "Hello, World!" );
    free(s);
}
END_TEST

START_TEST( can_set_and_peek )
{
    string_buffer_set(sb, strdup( "Hello" ));

    ck_assert_str_eq( string_buffer_peek(sb), "Hello" );

    string_buffer_delete( sb );
}
END_TEST

START_TEST( can_set_append_and_peek )
{
    string_buffer_set(sb, strdup( "Hello" ));
    string_buffer_append(sb, strdup( ", World!" ));

    ck_assert_str_eq( string_buffer_peek(sb), "Hello, World!" );

    string_buffer_delete( sb );
}
END_TEST

START_TEST( can_make_from_chars )
{
    string_buffer_t *sb1 = string_buffer_from_chars(strdup( "Hello" ));
    char *s;

    s = string_buffer_commit(sb1);
    ck_assert_str_eq( s, "Hello" );
    free(s);
}
END_TEST

START_TEST( can_append_to_sb_from_chars )
{
    string_buffer_t *sb1 = string_buffer_from_chars(strdup( "Hello" ));
    char *s;

    string_buffer_append(sb1, strdup( ", World!" ));

    s = string_buffer_commit(sb1);
    ck_assert_str_eq( s, "Hello, World!" );
    free(s);
}
END_TEST

START_TEST( can_append_multiple_times )
{
    char *s;

    string_buffer_set(sb, strdup( "Hello" ));
    string_buffer_append(sb, strdup( ", World!" ));
    string_buffer_append(sb, strdup( " World!" ));
    string_buffer_append(sb, strdup( " World!" ));
    string_buffer_append(sb, strdup( " World!" ));
    string_buffer_append(sb, strdup( " World!" ));

    s = string_buffer_commit(sb);
    ck_assert_str_eq( s, "Hello, World! World! World! World! World!" );
    free(s);
}
END_TEST

Suite *string_buffer_tests( void )
{
    Suite *s = suite_create( "string_buffer" );

    TCase *tc_core = tcase_create( "core" );
    tcase_add_checked_fixture( tc_core, setup, teardown );
    tcase_add_test( tc_core, can_set_and_commit );
    tcase_add_test( tc_core, can_set_append_and_commit );
    tcase_add_test( tc_core, can_set_and_peek );
    tcase_add_test( tc_core, can_set_append_and_peek );
    tcase_add_test( tc_core, can_make_from_chars );
    tcase_add_test( tc_core, can_append_to_sb_from_chars );
    tcase_add_test( tc_core, can_append_multiple_times );

    suite_add_tcase( s, tc_core );

    return s;
}
