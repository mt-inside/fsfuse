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
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <string.h>

#include "parser.h"

#include "fs2_constants.h"
#include "listing_list.h"
#include "string_buffer.h"


TRACE_DEFINE(parser)


struct _parser_t
{
    xmlParserCtxtPtr ctxt;
};


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

    xmlInitParser();


    return 0;
}

void parser_finalise (void)
{
    parser_trace("parser_finalise()\n");

    xmlCleanupParser();
}


parser_t *parser_new (void)
{
    parser_t *parser = malloc(sizeof(*parser));


    parser->ctxt = xmlCreatePushParserCtxt(NULL, NULL, NULL, 0, NULL);


    return parser;
}

void parser_delete (parser_t *parser)
{
    xmlFreeParserCtxt(parser->ctxt);

    free(parser);
}


int parser_consumer (void *ctxt, void *data, size_t len)
{
    int rc;
    parser_t *parser = (parser_t *)ctxt;


    rc = xmlParseChunk(parser->ctxt, data, len, 0);


    return rc;
}

static xmlDocPtr get_doc (xmlParserCtxtPtr ctxt)
{
    int rc;
    xmlDocPtr doc = NULL;


    /* Before we can get the (parsed) document, we indicate that the stream is
     * at the end. These are the magic runes: esentially telling libxml2 to
     * parse an empty packet, as one would get at the end of a TCP stream
     * (parser_consumer() never gets called on the empty buffer because they
     * fetcher doesn't give us one. */
    rc = xmlParseChunk(ctxt, NULL, 0, 1);

    if (!rc && ctxt->wellFormed)
    {
        doc = ctxt->myDoc;
    }


    return doc;
}

static xmlXPathObjectPtr parser_xhtml_xpath (xmlDocPtr doc, const char *xpath)
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
int parser_tryget_listing(
    parser_t *parser,
    indexnode_t *in,
    listing_list_t **lis
)
{
    xmlDocPtr doc;
    xmlXPathObjectPtr xpathObj;
    xmlNodeSetPtr nodes;
    listing_t *li;
    string_buffer_t *sb = string_buffer_new();
    int size, i;


    doc = get_doc(parser->ctxt);
    if (doc)
    {
        string_buffer_printf(sb, "//xhtml:div[@id='%s']/xhtml:a[@%s]", fs2_filelist_node_id, fs2_name_attribute_key);
        xpathObj = parser_xhtml_xpath(doc, string_buffer_peek(sb));
        if (xpathObj->type == XPATH_NODESET)
        {
            nodes = xpathObj->nodesetval;
            size = (nodes) ? nodes->nodeNr : 0;

            /* Enumerate the A elements */
            for (i = 0; i < size; i++)
            {
                li = filelist_entry_parse(in, (xmlElementPtr)nodes->nodeTab[i]);

                listing_list_set_item(*lis, i, li);
            }
        }

        xmlXPathFreeObject(xpathObj);
        string_buffer_delete(sb);
        xmlFreeDoc(doc);
    }


    return 0;
}

int parser_tryget_stats(
    parser_t *parser,
    unsigned long *files,
    unsigned long *bytes
)
{
    xmlDocPtr doc;
    xmlXPathObjectPtr xpathObj;
    xmlNodeSetPtr nodes;
    xmlNodePtr curNod = NULL;
    int size, i;
    char *id, *value;


    doc = get_doc(parser->ctxt);
    if (doc)
    {
        xpathObj = parser_xhtml_xpath(doc, "//xhtml:div[@id='general']/xhtml:span[@id]");
        if (xpathObj->type == XPATH_NODESET)
        {
            nodes = xpathObj->nodesetval;
            size = (nodes) ? nodes->nodeNr : 0;

            /* Enumerate the SPAN elements */
            for (i = 0; i < size; i++)
            {
                curNod = (xmlNodePtr)nodes->nodeTab[i];

                if (!strcmp((char *)curNod->name, "span"))
                {
                    id = (char *)xmlGetProp(curNod, BAD_CAST "id");
                    if (!strcmp(id, "file-count"))
                    {
                        value = (char *)xmlGetProp(curNod, BAD_CAST "value");

                        *files = atoll(value);

                        free(value);
                    }
                    else if (!strcmp(id, "total-size"))
                    {
                        value = (char *)xmlGetProp(curNod, BAD_CAST "value");

                        *bytes = atoll(value);

                        free(value);
                    }
                    free(id);
                }
            }
        }

        xmlXPathFreeObject(xpathObj);
        xmlFreeDoc(doc);
    }


    return 0;
}
