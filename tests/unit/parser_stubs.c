/*
 * Copyright (C) 2008-2013 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Parser stubs for testing.
 */

#include "common.h"

#include "parser_stubs.h"


parser_test_expectation_t stats_expected_results[] =
{
    { parser_xml_event_TAG_START, "html", NULL },
    { parser_xml_event_TAG_START, "body", NULL },
    { parser_xml_event_TAG_START, "div", "general" },
    { parser_xml_event_TAG_START, "span", "file-count" },
    { parser_xml_event_ATTRIBUTE, "value", "98740" },
    { parser_xml_event_TEXT,      "98740", NULL },
    { parser_xml_event_TAG_END,   "span", NULL },
    { parser_xml_event_TAG_START, "span", "indexnode-started" },
    { parser_xml_event_ATTRIBUTE, "value", "1367424501976" },
    { parser_xml_event_TEXT,      "Wed May 01 17:08:21 BST 2013", NULL },
    { parser_xml_event_TAG_END,   "span", NULL },
    { parser_xml_event_TAG_START, "br", NULL },
    { parser_xml_event_TAG_END,   "br", NULL },
    { parser_xml_event_TAG_START, "span", "total-size" },
    { parser_xml_event_ATTRIBUTE, "value", "4010079412955" },
    { parser_xml_event_TEXT,      "3.6 TiB", NULL },
    { parser_xml_event_TAG_END,   "span", NULL },
    { parser_xml_event_TAG_END,   "div", NULL },
    { parser_xml_event_TAG_END,   "body", NULL },
    { parser_xml_event_TAG_END,   "html", NULL }
};
size_t stats_expected_results_len = sizeof(stats_expected_results) / sizeof(stats_expected_results[0]);

parser_test_expectation_t filelist_expected_results[] =
{
    { parser_xml_event_TAG_START, "html", NULL },
    { parser_xml_event_TAG_START, "body", NULL },
    { parser_xml_event_TAG_START, "div", NULL },
    { parser_xml_event_TAG_START, "div", "fs2-filelist" },

    { parser_xml_event_TAG_START, "b", NULL },
    { parser_xml_event_TEXT,      "(1)", NULL },
    { parser_xml_event_TAG_END,   "b", NULL },
    { parser_xml_event_TAG_START, "a", NULL },
    { parser_xml_event_ATTRIBUTE, "fs2-alternativescount", "1" },
    { parser_xml_event_ATTRIBUTE, "fs2-clientalias", "3mpty-fileserver" },
    { parser_xml_event_ATTRIBUTE, "fs2-hash", "cea65d7062303adea837b6eeed2b5e63" },
    { parser_xml_event_ATTRIBUTE, "fs2-name", "docbook-xsl-1.78.0.tar.bz2" },
    { parser_xml_event_ATTRIBUTE, "fs2-size", "5011106" },
    { parser_xml_event_ATTRIBUTE, "fs2-type", "file" },
    { parser_xml_event_ATTRIBUTE, "href", "http://localhost:1337/download/cea65d7062303adea837b6eeed2b5e63" },
    { parser_xml_event_TEXT,      "docbook-xsl-1.78.0.tar.bz2", NULL },
    { parser_xml_event_TAG_END,   "a", NULL },
    { parser_xml_event_TAG_START, "span", NULL },
    { parser_xml_event_TEXT,      "(4.8 MiB)", NULL },
    { parser_xml_event_TAG_END,   "span", NULL },
    { parser_xml_event_TAG_START, "br", NULL },
    { parser_xml_event_TAG_END,   "br", NULL },

    { parser_xml_event_TAG_START, "b", NULL },
    { parser_xml_event_TEXT,      "(1)", NULL },
    { parser_xml_event_TAG_END,   "b", NULL },
    { parser_xml_event_TAG_START, "a", NULL },
    { parser_xml_event_ATTRIBUTE, "fs2-alternativescount", "1" },
    { parser_xml_event_ATTRIBUTE, "fs2-clientalias", "3mpty-fileserver" },
    { parser_xml_event_ATTRIBUTE, "fs2-hash", "8fd5667996cb59ddff343329ac29f9d2" },
    { parser_xml_event_ATTRIBUTE, "fs2-name", "Test-Pod-Coverage-1.08.tar.gz" },
    { parser_xml_event_ATTRIBUTE, "fs2-size", "6418" },
    { parser_xml_event_ATTRIBUTE, "fs2-type", "file" },
    { parser_xml_event_ATTRIBUTE, "href", "http://localhost:1337/download/8fd5667996cb59ddff343329ac29f9d2" },
    { parser_xml_event_TEXT,      "Test-Pod-Coverage-1.08.tar.gz", NULL },
    { parser_xml_event_TAG_END,   "a", NULL },
    { parser_xml_event_TAG_START, "span", NULL },
    { parser_xml_event_TEXT,      "(6.3 KiB)", NULL },
    { parser_xml_event_TAG_END,   "span", NULL },
    { parser_xml_event_TAG_START, "br", NULL },
    { parser_xml_event_TAG_END,   "br", NULL },

    { parser_xml_event_TAG_END,   "div", NULL },
    { parser_xml_event_TAG_END,   "div", NULL },
    { parser_xml_event_TAG_END,   "body", NULL },
    { parser_xml_event_TAG_END,   "html", NULL }
};
size_t filelist_expected_results_len = sizeof(filelist_expected_results) / sizeof(filelist_expected_results[0]);
