/*
 * API for the interface to the library used to parse e.g. metadata files.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#ifndef _INCLUDED_PARSER_H
#define _INCLUDED_PARSER_H

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include "common.h"
#include "direntry.h"


TRACE_DECLARE(parser)


extern int parser_init (void);
extern void parser_finalise (void);

extern xmlParserCtxtPtr parser_new (void);
extern void parser_delete (xmlParserCtxtPtr ctxt);

extern size_t parser_consumer (void *buf, size_t size, size_t nmemb, void *userp);
extern xmlDocPtr parser_done (xmlParserCtxtPtr ctxt);

extern xmlXPathObjectPtr parser_xhtml_xpath (xmlDocPtr doc, const char *xpath);

extern int parser_fetch_listing (
    const char * const url,
    listing_list_t **lis
);

#endif /* _INCLUDED_PARSER_H */
