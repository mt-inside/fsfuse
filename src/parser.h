/*
 * Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
 *
 * API for the interface to the library used to parse e.g. metadata files.
 */

#ifndef _INCLUDED_PARSER_H
#define _INCLUDED_PARSER_H

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include "common.h"
#include "listing.h"


TRACE_DECLARE(parser)
#define parser_trace(...) TRACE(parser,__VA_ARGS__)
#define parser_trace_indent() TRACE_INDENT(parser)
#define parser_trace_dedent() TRACE_DEDENT(parser)


extern int parser_init (void);
extern void parser_finalise (void);

extern xmlParserCtxtPtr parser_new (void);
extern void parser_delete (xmlParserCtxtPtr ctxt);

extern int parser_consumer (void *ctxt, void *data, size_t len);
extern xmlDocPtr parser_done (xmlParserCtxtPtr ctxt);

extern xmlXPathObjectPtr parser_xhtml_xpath (xmlDocPtr doc, const char *xpath);

extern int parser_fetch_listing (
    indexnode_t *in,
    const char * const url,
    listing_list_t **lis
);
extern int parser_tryfetch_stats (
    const char *url,
    unsigned long *files,
    unsigned long *bytes
);

#endif /* _INCLUDED_PARSER_H */
