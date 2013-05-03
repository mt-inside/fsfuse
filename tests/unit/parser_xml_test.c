/*
 * Copyright (C) 2008-2013 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * XML stream -> tag stream parser tests.
 */

#include "common.h"

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <check.h>
#include "tests.h"

#include "parser/parser_xml.h"


typedef struct
{
    parser_xml_event_t event;
    const char *text;
} test_expectation_t;

typedef struct
{
    test_expectation_t *expected_results;
    size_t i;
} test_ctxt_t;

static void feed_file( const char *name, parser_xml_t *xml )
{
    const char *path = test_isolate_file( name );
    int fd = open( path, O_RDONLY );
    ssize_t readed;
    char buf[ 16 ];

    while( (readed = read( fd, buf, sizeof(buf) )) )
    {
        parser_xml_consume( xml, buf, readed );
    }

    close( fd );
}

static void test_cb( void *ctxt, parser_xml_event_t event, char *text )
{
    test_ctxt_t *test_ctxt = (test_ctxt_t *)ctxt;

    ck_assert_int_eq( event, test_ctxt->expected_results[ test_ctxt->i ].event );
    ck_assert_str_eq( text,  test_ctxt->expected_results[ test_ctxt->i ].text  );
    (test_ctxt->i)++;
}

test_expectation_t stats_expected_results[] =
{
    { parser_xml_event_TAG_START, "html" },
    { parser_xml_event_TAG_START, "body" },
    { parser_xml_event_TAG_START, "div" },
    { parser_xml_event_TAG_START, "span" },
    { parser_xml_event_TEXT,      "Wed May 01 17:08:21 BST 2013" },
    { parser_xml_event_TAG_END,   "span" },
    { parser_xml_event_TAG_START, "br" },
    { parser_xml_event_TAG_END,   "br" },
    { parser_xml_event_TAG_END,   "div" },
    { parser_xml_event_TAG_END,   "body" },
    { parser_xml_event_TAG_END,   "html" }
};
START_TEST( can_parse_stats_page )
{
    /* Setup */
    test_ctxt_t *test_cb_ctxt = malloc( sizeof(*test_cb_ctxt) );
    test_cb_ctxt->expected_results = stats_expected_results;
    test_cb_ctxt-> i = 0;
    parser_xml_t *xml = parser_xml_new( &test_cb, test_cb_ctxt );

    /* Assert */
    feed_file( strdup( "stats.xml" ), xml );

    /* Teardown */
    parser_xml_delete( xml );
    free( test_cb_ctxt );
}
END_TEST

test_expectation_t filelist_expected_results[] =
{
    { parser_xml_event_TAG_START, "html" },
    { parser_xml_event_TAG_START, "body" },
    { parser_xml_event_TAG_START, "div" },
    { parser_xml_event_TAG_START, "div" },

    { parser_xml_event_TAG_START, "b" },
    { parser_xml_event_TEXT,      "(1)" },
    { parser_xml_event_TAG_END,   "b" },
    { parser_xml_event_TAG_START, "a" },
    { parser_xml_event_TEXT,      "docbook-xsl-1.78.0.tar.bz2" },
    { parser_xml_event_TAG_END,   "a" },
    { parser_xml_event_TAG_START, "span" },
    { parser_xml_event_TEXT,      "(4.8 MiB)" },
    { parser_xml_event_TAG_END,   "span" },
    { parser_xml_event_TAG_START, "br" },
    { parser_xml_event_TAG_END,   "br" },

    { parser_xml_event_TAG_START, "b" },
    { parser_xml_event_TEXT,      "(1)" },
    { parser_xml_event_TAG_END,   "b" },
    { parser_xml_event_TAG_START, "a" },
    { parser_xml_event_TEXT,      "Test-Pod-Coverage-1.08.tar.gz" },
    { parser_xml_event_TAG_END,   "a" },
    { parser_xml_event_TAG_START, "span" },
    { parser_xml_event_TEXT,      "(6.3 KiB)" },
    { parser_xml_event_TAG_END,   "span" },
    { parser_xml_event_TAG_START, "br" },
    { parser_xml_event_TAG_END,   "br" },

    { parser_xml_event_TAG_END,   "div" },
    { parser_xml_event_TAG_END,   "div" },
    { parser_xml_event_TAG_END,   "body" },
    { parser_xml_event_TAG_END,   "html" }
};
START_TEST( can_parse_filelist_page )
{
    /* Setup */
    test_ctxt_t *test_cb_ctxt = malloc( sizeof(*test_cb_ctxt) );
    test_cb_ctxt->expected_results = filelist_expected_results;
    test_cb_ctxt-> i = 0;
    parser_xml_t *xml = parser_xml_new( &test_cb, test_cb_ctxt );

    /* Assert */
    feed_file( strdup( "filelist.xml" ), xml );

    /* Teardown */
    parser_xml_delete( xml );
    free( test_cb_ctxt );
}
END_TEST

Suite *parser_xml_tests( void )
{
    Suite *s = suite_create( "parser_xml" );

    TCase *tc_parsing = tcase_create( "parsing" );
    tcase_add_test( tc_parsing, can_parse_stats_page );
    tcase_add_test( tc_parsing, can_parse_filelist_page );

    suite_add_tcase( s, tc_parsing );

    return s;
}
