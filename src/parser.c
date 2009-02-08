/*
 * Interface to the library used to parse e.g. metadata files.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#include <assert.h>

#include "common.h"
#include "parser.h"
#include "fetcher.h"


TRACE_DEFINE(parser)


/* ========================================================================== */
/* Externs                                                                    */
/* ========================================================================== */
int parser_init (void)
{
    parser_trace("parser_init()\n");

    xmlInitParser();
    LIBXML_TEST_VERSION

#ifndef LIBXML_PUSH_ENABLED
#error libxml not comiled with push mode support
#endif


    return 0;
}

void parser_finalise (void)
{
    parser_trace("parser_finalise()\n");

    xmlCleanupParser();
}


xmlParserCtxtPtr parser_new (void)
{
    xmlParserCtxtPtr ctxt;


    ctxt = xmlCreatePushParserCtxt(NULL, NULL, NULL, 0, NULL);
    assert(ctxt);


    return ctxt;
}

void parser_delete (xmlParserCtxtPtr ctxt)
{
    xmlFreeParserCtxt(ctxt);
}


size_t parser_consumer (void *buf, size_t size, size_t nmemb, void *userp)
{
    int rc;
    size_t len = size * nmemb;
    xmlParserCtxtPtr ctxt = (xmlParserCtxtPtr)((fetcher_cb_data_t *)userp)->cb_data;


    parser_trace("parser_consumer(size==%zd, nmemb==%zd, userp==%p)\n",
                 size, nmemb, userp);
    parser_trace_indent();


    rc = xmlParseChunk(ctxt, buf, len, 0);


    parser_trace_dedent();


    return (!rc) ? len : 0;
}

xmlDocPtr parser_done (xmlParserCtxtPtr ctxt)
{
    int rc;
    xmlDocPtr doc = NULL;


    parser_trace("parser_done()\n");

    rc = xmlParseChunk(ctxt, NULL, 0, 1);

    if (!rc && ctxt->wellFormed)
    {
        doc = ctxt->myDoc;
    }


    return doc;
}

xmlXPathObjectPtr parser_xhtml_xpath (xmlDocPtr doc, const char *xpath)
{
    xmlXPathContextPtr xpathCtxt;
    xmlXPathObjectPtr xpathObj;


    xpathCtxt = xmlXPathNewContext(doc);
    xpathCtxt->node = xmlDocGetRootElement(doc);
    xmlXPathRegisterNs(xpathCtxt, BAD_CAST "xhtml", BAD_CAST "http://www.w3.org/1999/xhtml");

    xpathObj = xmlXPathEvalExpression(BAD_CAST xpath, xpathCtxt);

    xmlXPathFreeContext(xpathCtxt);


    return xpathObj;
}
