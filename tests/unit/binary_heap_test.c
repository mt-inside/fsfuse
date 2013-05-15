/*
 * Copyright (C) 2008-2013 Matthew Turner.
 *
 * This procram is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Binary heap tests.
 */

#include "common.h"

#include <stdlib.h>
#include <string.h>

#include <check.h>
#include "tests.h"

#include "binary_heap.h"


START_TEST( empty_is_sane )
{
    /* Setup */
    binary_heap_t *heap = binary_heap_new( );

    /* Assert - check some of the values */
    fail_unless( heap != NULL, "heap should be non-null" );

    /* Teardown */
    binary_heap_delete( heap );
}
END_TEST

START_TEST( singleton_is_sane )
{
    const int key_in = 0;
    int value_in;
    int key_out;
    int *value_out;

    /* Setup */
    binary_heap_t *heap = binary_heap_new( );
    srandom( time( NULL ) );
    value_in = random( );

    /* Assert - check some of the values */
    binary_heap_add( heap, key_in, &value_in );

    binary_heap_trypop( heap, &key_out, (void **)&value_out );
    ck_assert_int_eq( key_out, key_in );
    ck_assert_int_eq( *value_out, value_in );

    /* Teardown */
    binary_heap_delete( heap );
}
END_TEST

START_TEST( in_order )
{
    int pairs[5][2];
    int key_out;
    int *value_out;
    unsigned i;

    /* Setup */
    binary_heap_t *heap = binary_heap_new( );

    srandom( time( NULL ) );
    for( i = 0; i < sizeof(pairs) / sizeof(pairs[0]); i++ )
    {
        pairs[ i ][ 0 ] = i; /* key */
        pairs[ i ][ 1 ] = random( );
        binary_heap_add( heap, pairs[ i ][ 0 ], &pairs[ i ][ 1 ] );
    }

    /* Assert - check some of the values */
    for( i = 0; i < sizeof(pairs) / sizeof(pairs[0]); i++ )
    {
        binary_heap_trypop( heap, &key_out, (void **)&value_out );
        ck_assert_int_eq( key_out, pairs[ i ][ 0 ] );
        ck_assert_int_eq( *value_out, pairs[ i ][ 1 ] );
    }

    /* Teardown */
    binary_heap_delete( heap );
}
END_TEST

START_TEST( in_order_duplicates )
{
    int pairs[5][2];
    int key_out;
    int *value_out;
    unsigned i;

    /* Setup */
    binary_heap_t *heap = binary_heap_new( );

    for( i = 0; i < sizeof(pairs) / sizeof(pairs[0]); i++ )
    {
        pairs[ i ][ 0 ] = i / 2; /* key */
        binary_heap_add( heap, pairs[ i ][ 0 ], &pairs[ i ][ 1 ] );
    }

    /* Assert - check some of the values */
    for( i = 0; i < sizeof(pairs) / sizeof(pairs[0]); i++ )
    {
        binary_heap_trypop( heap, &key_out, (void **)&value_out );
        ck_assert_int_eq( key_out, pairs[ i ][ 0 ] );
    }

    /* Teardown */
    binary_heap_delete( heap );
}
END_TEST

START_TEST( reverse_order )
{
    int pairs[5][2];
    int key_out;
    int *value_out;
    int i;

    /* Setup */
    binary_heap_t *heap = binary_heap_new( );

    srandom( time( NULL ) );
    for( i = (int)( sizeof(pairs) / sizeof(pairs[0]) ) - 1; i >= 0; i-- )
    {
        pairs[ i ][ 0 ] = i; /* key */
        pairs[ i ][ 1 ] = random( );
        binary_heap_add( heap, pairs[ i ][ 0 ], &pairs[ i ][ 1 ] );
    }

    /* Assert - check some of the values */
    for( i = 0; i < (int)( sizeof(pairs) / sizeof(pairs[0]) ); i++ )
    {
        binary_heap_trypop( heap, &key_out, (void **)&value_out );
        ck_assert_int_eq( key_out, pairs[ i ][ 0 ] );
        ck_assert_int_eq( *value_out, pairs[ i ][ 1 ] );
    }

    /* Teardown */
    binary_heap_delete( heap );
}
END_TEST

START_TEST( reverse_order_duplicates )
{
    int pairs[5][2];
    int key_out;
    int *value_out;
    int i;

    /* Setup */
    binary_heap_t *heap = binary_heap_new( );

    for( i = (int)( sizeof(pairs) / sizeof(pairs[0]) ) - 1; i >= 0; i-- )
    {
        pairs[ i ][ 0 ] = i / 2; /* key */
        binary_heap_add( heap, pairs[ i ][ 0 ], &pairs[ i ][ 1 ] );
    }

    /* Assert - check some of the values */
    for( i = 0; i < (int)( sizeof(pairs) / sizeof(pairs[0]) ); i++ )
    {
        binary_heap_trypop( heap, &key_out, (void **)&value_out );
        ck_assert_int_eq( key_out, pairs[ i ][ 0 ] );
    }

    /* Teardown */
    binary_heap_delete( heap );
}
END_TEST

START_TEST( funny_order1 )
{
    int pairs[5][2];
    int key_out;
    int *value_out;
    int i;

    /* Setup */
    binary_heap_t *heap = binary_heap_new( );

    pairs[ 0 ][ 0 ] = 0;
    pairs[ 1 ][ 0 ] = 3;
    pairs[ 2 ][ 0 ] = 4;
    pairs[ 3 ][ 0 ] = 2;
    pairs[ 4 ][ 0 ] = 1;

    srandom( time( NULL ) );
    for( i = 0; i < (int)( sizeof(pairs) / sizeof(pairs[0]) ); i++ )
    {
        binary_heap_add( heap, pairs[ i ][ 0 ], &pairs[ i ][ 1 ] );
    }

    /* Assert - check some of the values */
    for( i = 0; i < (int)( sizeof(pairs) / sizeof(pairs[0]) ); i++ )
    {
        binary_heap_trypop( heap, &key_out, (void **)&value_out );
        ck_assert_int_eq( key_out, i );
    }

    /* Teardown */
    binary_heap_delete( heap );
}
END_TEST

START_TEST( funny_order2 )
{
    int pairs[5][2];
    int key_out;
    int *value_out;
    int i;

    /* Setup */
    binary_heap_t *heap = binary_heap_new( );

    pairs[ 0 ][ 0 ] = 3;
    pairs[ 1 ][ 0 ] = 2;
    pairs[ 2 ][ 0 ] = 0;
    pairs[ 3 ][ 0 ] = 1;
    pairs[ 4 ][ 0 ] = 4;

    srandom( time( NULL ) );
    for( i = 0; i < (int)( sizeof(pairs) / sizeof(pairs[0]) ); i++ )
    {
        binary_heap_add( heap, pairs[ i ][ 0 ], &pairs[ i ][ 1 ] );
    }

    /* Assert - check some of the values */
    for( i = 0; i < (int)( sizeof(pairs) / sizeof(pairs[0]) ); i++ )
    {
        binary_heap_trypop( heap, &key_out, (void **)&value_out );
        ck_assert_int_eq( key_out, i );
    }

    /* Teardown */
    binary_heap_delete( heap );
}
END_TEST


Suite *binary_heap_tests( void )
{
    Suite *s = suite_create( "binary_heap" );

    TCase *tc_simple = tcase_create( "simple" );
    tcase_add_test( tc_simple, empty_is_sane );
    tcase_add_test( tc_simple, singleton_is_sane );
    tcase_add_test( tc_simple, in_order );
    tcase_add_test( tc_simple, in_order_duplicates );
    tcase_add_test( tc_simple, reverse_order );
    tcase_add_test( tc_simple, reverse_order_duplicates );
    tcase_add_test( tc_simple, funny_order1 );
    tcase_add_test( tc_simple, funny_order2 );

    suite_add_tcase( s, tc_simple );

    return s;
}
