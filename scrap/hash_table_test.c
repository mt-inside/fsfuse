/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Hash module unit tests
 */

#include <string.h>

#include "common.h"
#include "tests.h"
#include "hash_table.h"


static const char *key1 = "key one",
                  *key2 = "key two",
                  *key3 = "key three";

static const char *value1 = "value one",
                  *value2 = "value two",
                  *value3 = "value three";


static hash_table_t *tbl;

static void setup( void )
{
    tbl = hash_table_new( 0.5f, 1.0f );
}

static void teardown( void )
{
    hash_table_delete( tbl );
}


START_TEST( empty_table )
{
    fail_unless( tbl != NULL, "new hash table should be non-null" );
}
END_TEST

START_TEST( can_add_and_find_entries )
{
    hash_table_add(tbl, key1, (void *)value1);
    hash_table_add(tbl, key2, (void *)value2);
    hash_table_add(tbl, key3, (void *)value3);

    ck_assert_str_eq( (char *)hash_table_find(tbl, key1), value1 );
    ck_assert_str_eq( (char *)hash_table_find(tbl, key2), value2 );
    ck_assert_str_eq( (char *)hash_table_find(tbl, key3), value3 );

    hash_table_del(tbl, key1);
    hash_table_del(tbl, key2);
    hash_table_del(tbl, key3);
}
END_TEST

START_TEST( can_remove_entries )
{
    hash_table_add(tbl, key1, (void *)value1);
    hash_table_add(tbl, key2, (void *)value2);
    hash_table_add(tbl, key3, (void *)value3);

    fail_unless(hash_table_del(tbl, key1) == 1, "should be able to find this entry to delete");

    fail_unless(hash_table_find(tbl, key1) == NULL, "shouldn't be able to find deleted entry");
    ck_assert_str_eq( (char *)hash_table_find(tbl, key2), value2 );
    ck_assert_str_eq( (char *)hash_table_find(tbl, key3), value3 );

    fail_unless(hash_table_del(tbl, key1) == 0, "shouldn't be able to delete already deleted entry");
    fail_unless(hash_table_del(tbl, key2) == 1, "should still be able to delete this entry");
    fail_unless(hash_table_del(tbl, key3) == 1, "should still be able to delete this entry");

    fail_unless(hash_table_find(tbl, key1) == NULL, "all entries should be gone by now");
    fail_unless(hash_table_find(tbl, key2) == NULL, "all entries should be gone by now");
    fail_unless(hash_table_find(tbl, key3) == NULL, "all entries should be gone by now");
}
END_TEST

START_TEST( can_remove_and_readd_entries )
{
    hash_table_add(tbl, key1, (void *)value1);
    hash_table_add(tbl, key2, (void *)value2);
    hash_table_add(tbl, key3, (void *)value3);

    fail_unless(hash_table_del(tbl, key1) == 1, "should be able to delete this entry");
    fail_unless(hash_table_del(tbl, key2) == 1, "should be able to delete this entry");
    fail_unless(hash_table_del(tbl, key3) == 1, "should be able to delete this entry");

    fail_unless(hash_table_find(tbl, key1) == NULL, "all entries should be gone by now");
    fail_unless(hash_table_find(tbl, key2) == NULL, "all entries should be gone by now");
    fail_unless(hash_table_find(tbl, key3) == NULL, "all entries should be gone by now");

    hash_table_add(tbl, key1, (void *)value1);
    hash_table_add(tbl, key2, (void *)value2);
    hash_table_add(tbl, key3, (void *)value3);

    ck_assert_str_eq( (char *)hash_table_find(tbl, key1), value1 );
    ck_assert_str_eq( (char *)hash_table_find(tbl, key2), value2 );
    ck_assert_str_eq( (char *)hash_table_find(tbl, key3), value3 );

    hash_table_del(tbl, key1);
    hash_table_del(tbl, key2);
    hash_table_del(tbl, key3);
}
END_TEST

Suite *hash_table_tests( void )
{
    Suite *s = suite_create( "hash_table" );

    TCase *tc_core = tcase_create( "core" );
    tcase_add_checked_fixture( tc_core, setup, teardown );

    tcase_add_test( tc_core, empty_table );
    tcase_add_test( tc_core, can_add_and_find_entries );
    tcase_add_test( tc_core, can_remove_entries );
    tcase_add_test( tc_core, can_remove_and_readd_entries );
    /* TODO: iterator tests */

    suite_add_tcase( s, tc_core );

    return s;
}
