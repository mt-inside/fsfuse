/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * Class that parses indexnode stats pages.
 */

#ifndef _INCLUDED_PARSER_STATS_H
#define _INCLUDED_PARSER_STATS_H

#include "common.h"

#include "parser/parser_xml.h" //TODO: I can go when xml_cb is internal */


typedef struct _parser_stats_t parser_stats_t;

typedef void (*parser_stats_cb_t)( void *ctxt, unsigned long files, unsigned long bytes );


extern parser_stats_t *parser_stats_new( parser_stats_cb_t cb, void *ctxt );
extern void parser_stats_delete( parser_stats_t *parser );
extern int parser_stats_consume( parser_stats_t *parser, void *data, size_t len );

void stats_xml_cb( void *ctxt, parser_xml_event_t event, char *text, char *id ); //TODO: should be static, fix tests

#endif /* _INCLUDED_PARSER_STATS_H */
