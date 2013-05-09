/*
 * Copyright (C) 2008-2013 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * tag stream -> business model parser tests.
 */

#include "common.h"

#include <stdlib.h>

#include <check.h>
#include "tests.h"
#include "parser_stubs.h"

#include "parser_stats.h"


typedef struct
{
    int called;
} test_ctxt_t;


static void feed_tags( parser_test_expectation_t *tags, size_t tags_len, parser_stats_t *parser )
{
    unsigned i;


    for( i = 0; i < tags_len; i++ )
    {
        stats_xml_cb( parser, tags[i].event, strdup( tags[i].text ), tags[i].id ? strdup( tags[i].id ) : NULL );
    }
}

static void stats_cb( void *ctxt, unsigned long files, unsigned long bytes )
{
    test_ctxt_t *test_cb_ctxt = (test_ctxt_t *)ctxt;

    test_cb_ctxt->called = 1;

    ck_assert_int_eq( files, 98740 );
    ck_assert_int_eq( bytes, 4010079412955 );
}

START_TEST( can_parse_stats_page )
{
    /* Setup */
    test_ctxt_t *test_cb_ctxt = malloc( sizeof(*test_cb_ctxt) );
    test_cb_ctxt->called = 0;
    parser_stats_t *parser = parser_stats_new( &stats_cb, test_cb_ctxt );

    /* Assert */
    feed_tags( stats_expected_results, stats_expected_results_len, parser );
    assert( test_cb_ctxt->called == 1 );

    /* Teardown */
    parser_stats_delete( parser );
    free( test_cb_ctxt );
}
END_TEST

#if 0
START_TEST( can_parse_filelist_page )
{
    /* Setup */
    test_ctxt_t *test_cb_ctxt = malloc( sizeof(*test_cb_ctxt) );
    test_cb_ctxt->called = 0;
    parser_filelist_t *parser = parser_filelist_new( &stats_cb, NULL );

    /* Assert */
    feed_tags( filelist_expected_results, parser );
    assert( test_cb_ctxt->called == 1 );

    /* Teardown */
    parser_filelist_delete( parser );
    free( test_cb_ctxt );
}
END_TEST
#endif

Suite *parser_tests( void )
{
    Suite *s = suite_create( "parser" );

    TCase *tc_parsing = tcase_create( "parsing" );
    tcase_add_test( tc_parsing, can_parse_stats_page );
    //tcase_add_test( tc_parsing, can_parse_filelist_page );

    suite_add_tcase( s, tc_parsing );

    return s;
}
