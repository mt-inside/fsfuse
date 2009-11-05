/*
 * Interface to the library used to parse e.g. metadata files.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include "common.h"
#include "parser.h"
#include "fetcher.h"


TRACE_DEFINE(parser)


static int filelist_entries_parse (
    xmlNodeSetPtr nodes,
    listing_list_t **lis
);
static int filelist_entry_parse (
    xmlElementPtr node,
    listing_t *li
);


/* ========================================================================== */
/* Externs                                                                    */
/* ========================================================================== */
int parser_init (void)
{
    parser_trace("parser_init()\n");

    LIBXML_TEST_VERSION

#ifndef LIBXML_PUSH_ENABLED
#error libxml not comiled with push mode support
#endif


    return 0;
}

void parser_finalise (void)
{
    parser_trace("parser_finalise()\n");
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

int parser_fetch_listing (
    const char * const url,
    listing_list_t **lis
)
{
    int rc;
    xmlParserCtxtPtr parser = parser_new();
    xmlXPathObjectPtr xpathObj;
    xmlDocPtr doc;


    assert(url); assert(*url);

    parser_trace("parser_fetch_listing(url: %s)\n", url);
    parser_trace_indent();

    rc = fetcher_fetch_internal(url, NULL, (curl_write_callback)&parser_consumer, (void *)parser);

    if (rc == 0 && lis)
    {
        /* The fetcher has returned, so that's all the document.
         * Indicate to the parser that that's it */
        doc = parser_done(parser);

        xpathObj = parser_xhtml_xpath(doc, "//xhtml:div[@id='fs2-filelist']/xhtml:a[@fs2-name]");
        if (xpathObj->type == XPATH_NODESET)
        {
            rc = filelist_entries_parse(xpathObj->nodesetval, lis);
        }

        xmlXPathFreeObject(xpathObj);
        xmlFreeDoc(doc);
    }

    parser_delete(parser);

    parser_trace_dedent();


    return rc;
}

/* Parse a nodeset representing the A tags in an fs2-filelist,
 * building direntries */
static int filelist_entries_parse (
    xmlNodeSetPtr nodes,
    listing_list_t **lis_out
)
{
    listing_t *li;
    listing_list_t *lis = NULL;
    int rc = 0, size, i;


    size = (nodes) ? nodes->nodeNr : 0;

    parser_trace("filelist_entries_parse(): enumerating %d nodes\n", size);
    parser_trace_indent();

    if (size)
    {
        lis = (listing_list_t *)malloc(sizeof(listing_list_t));
        lis->count = size;
        lis->items = (listing_t **)malloc(size * sizeof(listing_t *));

        /* Enumerate the A elements */
        for (i = 0; i < size && !rc; i++)
        {
            if (!(li = listing_new(CALLER_INFO_ONLY)))
            {
                rc = -EIO;
                break;
            }

            rc = filelist_entry_parse((xmlElementPtr)nodes->nodeTab[i],
                                      li);

            lis->items[i] = li;
        }
    }

    *lis_out = lis;

    parser_trace_dedent();


    return rc;
}

static int filelist_entry_parse (
    xmlElementPtr node,
    listing_t *li
)
{
    xmlAttributePtr curAttr = NULL;


    assert(node);
    assert(li);

    parser_trace("filelist_entry_parse(element content==%s)\n", node->children->content);
    parser_trace_indent();

    if (node->type != XML_ELEMENT_NODE ||
        strcmp((char *)node->name, "a"))
    {
        return -EIO;
    }

    /* Enumerate the element's attributes */
    curAttr = (xmlAttributePtr)node->attributes;
    while (curAttr)
    {
        if (curAttr->type == XML_ATTRIBUTE_NODE &&
            curAttr->children &&
            curAttr->children->type == XML_TEXT_NODE &&
            !curAttr->children->next)
        {
            /* ignore what the indexnode says the path is for now */
            if (strcmp((char *)curAttr->name, "fs2-path"))
            {
                parser_trace("Attribute %s == %s\n", curAttr->name, curAttr->children->content);
                listing_attribute_add(li, (char *)curAttr->name, (char *)curAttr->children->content);
            }
        }

        curAttr = (xmlAttributePtr)curAttr->next;
    }

    parser_trace_dedent();


    return 0;
}
