/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * Parser stubs for testing.
 */

#ifndef _INCLUDED_PARSER_STUBS_H
#define _INCLUDED_PARSER_STUBS_H

#include "common.h"

#include "parser/parser_xml.h"

typedef struct
{
    parser_xml_event_t event;
    const char *text;
    const char *id;
} parser_test_expectation_t;


extern parser_test_expectation_t stats_expected_results[];
extern size_t stats_expected_results_len;

extern parser_test_expectation_t filelist_expected_results[];
extern size_t filelist_expected_results_len;

#endif /* _INCLUDED_PARSER_STUBS_H */
