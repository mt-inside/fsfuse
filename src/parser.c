/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Interface to the library used to parse e.g. metadata files.
 */

#include "common.h"

#include <assert.h>
#include <errno.h>
#include <string.h>

#include "parser.h"

#include "fetcher.h"
#include "fs2_constants.h"
#include "string_buffer.h"


TRACE_DEFINE(parser)


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


int parser_consumer (void *ctxt, void *data, size_t len)
{
    int rc;
    xmlParserCtxtPtr parser_ctxt = (xmlParserCtxtPtr)ctxt;


    rc = xmlParseChunk(parser_ctxt, data, len, 0);


    return rc;
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

static listing_t *filelist_entry_parse (
    indexnode_t *in,
    xmlElementPtr node
)
{
    xmlAttributePtr curAttr = NULL;
    listing_t *li = NULL;
    const char *key, *value,
               *name, *hash, *type, *href, *client;
    off_t size; unsigned long link_count;


    assert(node);

    parser_trace("filelist_entry_parse(element content==%s)\n", node->children->content);
    parser_trace_indent();

    if (node->type != XML_ELEMENT_NODE ||
        strcmp((char *)node->name, "a"))
    {
        return NULL;
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
            key = (const char *)curAttr->name;
            value = (const char *)curAttr->children->content;

            if (!strcmp(key, fs2_name_attribute_key))
            {
                name = strdup(value);
            }
            else if (!strcmp(key, fs2_hash_attribute_key))
            {
                hash = strdup(value);
            }
            else if (!strcmp(key, fs2_type_attribute_key))
            {
                type = strdup(value);
            }
            else if (!strcmp(key, fs2_size_attribute_key))
            {
                size = atoll(value);
            }
            else if (!strcmp(key, fs2_linkcount_attribute_key) ||
                     !strcmp(key, fs2_alternativescount_attribute_key))
            {
                link_count = atol(value);
            }
            else if (!strcmp(key, fs2_href_attribute_key))
            {
                href = strdup(value);
            }
            else if (!strcmp(key, fs2_clientalias_attribute_key))
            {
                client = strdup(value);
            }
            else if (!strcmp(key, fs2_path_attribute_key))
            {
                /* ignore what the indexnode says the path is for now */
            }
            else
            {
                listing_trace("Unknown attribute %s == %s\n", key, value);
            }
        }

        curAttr = (xmlAttributePtr)curAttr->next;
    }

    li = listing_new( CALLER_INFO in, hash, name, type, size, link_count, href, client );

    parser_trace_dedent();


    return li;
}

/* Parse a nodeset representing the A tags in an fs2-filelist,
 * building direntries */
static int filelist_entries_parse (
    indexnode_t *in,
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

    lis = listing_list_new(size);

    /* Enumerate the A elements */
    for (i = 0; i < size && !rc; i++)
    {
        li = filelist_entry_parse(in, (xmlElementPtr)nodes->nodeTab[i]);

        listing_list_set_item(lis, i, li);
        listing_delete(CALLER_INFO li);
    }

    *lis_out = lis;

    parser_trace_dedent();


    return rc;
}

int parser_fetch_listing (
    indexnode_t *in,
    const char * const url,
    listing_list_t **lis
)
{
    int rc;
    xmlParserCtxtPtr parser = parser_new();
    xmlXPathObjectPtr xpathObj;
    xmlDocPtr doc;
    string_buffer_t *sb = string_buffer_new();


    assert(url); assert(*url);

    parser_trace("parser_fetch_listing(url: %s)\n", url);
    parser_trace_indent();

    rc = fetch(
        url,
        NULL, NULL,
        (fetcher_body_cb_t)&parser_consumer, (void *)parser,
        0,
        NULL
    );

    if (rc == 0 && lis)
    {
        /* The fetcher has returned, so that's all the document.
         * Indicate to the parser that that's it */
        doc = parser_done(parser);

        string_buffer_printf(sb, "//xhtml:div[@id='%s']/xhtml:a[@%s]", fs2_filelist_node_id, fs2_name_attribute_key);
        xpathObj = parser_xhtml_xpath(doc, string_buffer_peek(sb));
        if (xpathObj->type == XPATH_NODESET)
        {
            rc = filelist_entries_parse(in, xpathObj->nodesetval, lis);
        }

        xmlXPathFreeObject(xpathObj);
        string_buffer_delete(sb);
        xmlFreeDoc(doc);
    }

    parser_delete(parser);

    parser_trace_dedent();


    return rc;
}
