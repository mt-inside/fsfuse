/*
 * XML file based configuration component with a simple API.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#include "common.h"
#include "config.h"

#include <string.h>
#include <assert.h>

#include <libxml/parser.h>
#include <libxml/xpath.h>


static char *config_path = NULL;


static void config_items_free (void);
static char *xpath_get (const char *xpath, xmlXPathContextPtr xpathCtxt);


int config_init (void)
{
    if (!config_path) config_path = strdup("fsfuse.conf");


    return 0;
}

void config_finalise (void)
{
    config_items_free();

    free(config_path);
}


int config_read (void)
{
    config_item *item;
    char *val;
    xmlDocPtr doc;
    xmlXPathContextPtr xpathCtxt;


    /* Read the config file */
    doc = xmlParseFile(config_path_get());

    if (doc)
    {
        xpathCtxt = xmlXPathNewContext(doc);

        for (item = config_items; item->symbol; item++)
        {
            val = xpath_get(item->xpath, xpathCtxt);
            if (val)
            {
                switch (item->type)
                {
                    case config_item_type_STRING:
                        if (item->runtime) free(*((char **)item->symbol));

                        *((char **)item->symbol) = strdup(val);
                        item->runtime = 1;

                        break;

                    case config_item_type_NUMBER:
                        *((int *)item->symbol) = strtoul(val, NULL, 0);
                        break;
                }
            }
        }

        xmlXPathFreeContext(xpathCtxt);
        xmlFreeDoc(doc);
    }
    else
    {
        trce("Unable to read / parse config file %s\n", config_path_get());
    }


    return 0;
}

static void config_items_free (void)
{
    config_item *item;


    for (item = config_items; item->symbol; item++)
    {
        switch (item->type)
        {
            case config_item_type_STRING:
                if (item->runtime) free(*((char **)item->symbol));
                break;

            case config_item_type_NUMBER:
                break;
        }
    }
}

static char *xpath_get (const char *xpath, xmlXPathContextPtr xpathCtxt)
{
    xmlXPathObjectPtr xpathObj;
    xmlNodeSetPtr nodeset;
    xmlNodePtr node;
    char *ret = NULL;


    xpathObj = xmlXPathEvalExpression(BAD_CAST xpath, xpathCtxt);
    if (xpathObj && xpathObj->type == XPATH_NODESET)
    {
        nodeset = xpathObj->nodesetval;
        if (nodeset && nodeset->nodeNr == 1)
        {
            node = (xmlNodePtr)nodeset->nodeTab[0];
            if (node && node->type == XML_TEXT_NODE)
            {
                ret = (char *)node->content;
            }
        }
    }

    xmlXPathFreeObject(xpathObj);


    return ret;
}

char *config_path_get (void)
{
    return config_path;
}

void config_path_set (char *config_path_new)
{
    if (config_path) free(config_path);
    config_path = config_path_new;
}
