/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * Class that parses indexnode direntry listing pages.
 */

#ifndef _INCLUDED_PARSER_FILELIST_H
#define _INCLUDED_PARSER_FILELIST_H

#include "common.h"

#include "nativefs.h"
#include "parser/parser_xml.h" //TODO: I can go when xml_cb is internal */


typedef struct _parser_filelist_t parser_filelist_t;


extern parser_filelist_t *parser_filelist_new( nativefs_entry_found_cb_t cb, void *ctxt );
extern void parser_filelist_delete( parser_filelist_t *parser );
extern int parser_filelist_consume( parser_filelist_t *parser, void *data, size_t len );

void filelist_xml_cb( void *ctxt, parser_xml_event_t event, char *text, char *id ); //TODO: should be static, fix tests

#endif /* _INCLUDED_PARSER_FILELIST_H */
