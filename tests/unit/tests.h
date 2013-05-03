/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * Unit tests header.
 */

#include <check.h>


extern Suite *config_tests( void );
extern Suite *indexnode_tests( void );
extern Suite *indexnodes_list_tests( void );
extern Suite *parser_xml_tests( void );
extern Suite *proto_indexnode_tests( void );
extern Suite *ref_count_tests( void );
extern Suite *string_buffer_tests( void );

extern char *test_isolate_file( const char *name );

//extern void http_test( void );
//extern void uri_test( void );
//extern void utils_test( void );
