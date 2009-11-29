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
static char **xpath_get_array (const char *xpath, xmlXPathContextPtr xpathCtxt);
static void string_collection_free (char **strcol);


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
    char *val, **val_array;
    xmlDocPtr doc;
    xmlXPathContextPtr xpathCtxt;


    /* Read the config file */
    doc = xmlParseFile(config_path_get());

    if (doc)
    {
        xpathCtxt = xmlXPathNewContext(doc);

        for (item = config_items; item->symbol; item++)
        {
            switch (item->type)
            {
                case config_item_type_STRING:
                    val = xpath_get(item->xpath, xpathCtxt);

                    if (val)
                    {
                        if (item->runtime) free(*((char **)item->symbol));

                        *((char **)item->symbol) = strdup(val);
                        item->runtime = 1;
                    }

                    break;

                case config_item_type_INTEGER:
                    val = xpath_get(item->xpath, xpathCtxt);

                    if (val)
                    {
                        *((int *)item->symbol) = strtoul(val, NULL, 0);
                    }

                    break;

                case config_item_type_FLOAT:
                    val = xpath_get(item->xpath, xpathCtxt);

                    if (val)
                    {
                        *((double *)item->symbol) = strtod(val, NULL);
                    }

                    break;

                case config_item_type_STRING_COLLECTION:
                    val_array = xpath_get_array(item->xpath, xpathCtxt);

                    if (val_array)
                    {
                        if (item->runtime) string_collection_free(*((char ***)item->symbol));

                        *((char ***)item->symbol) = val_array;
                        item->runtime = 1;
                    }

                    break;
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

            case config_item_type_INTEGER:
            case config_item_type_FLOAT:
                break;

            case config_item_type_STRING_COLLECTION:
                if (item->runtime) string_collection_free(*((char ***)item->symbol));
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

static char **xpath_get_array (const char *xpath, xmlXPathContextPtr xpathCtxt)
{
    int i;
    xmlXPathObjectPtr xpathObj;
    xmlNodeSetPtr nodeset;
    xmlNodePtr node;
    char **ret = NULL;


    xpathObj = xmlXPathEvalExpression(BAD_CAST xpath, xpathCtxt);
    if (xpathObj && xpathObj->type == XPATH_NODESET)
    {
        nodeset = xpathObj->nodesetval;
        if (nodeset)
        {
            ret = (char **)malloc((nodeset->nodeNr + 1) * sizeof(char *));

            for (i = 0; i < nodeset->nodeNr; i++)
            {
                node = (xmlNodePtr)nodeset->nodeTab[i];
                if (node && node->type == XML_TEXT_NODE)
                {
                    ret[i] = strdup((char *)node->content);
                }
            }
            ret[nodeset->nodeNr] = NULL;
        }
    }

    xmlXPathFreeObject(xpathObj);


    /* If we didn't get any real results, allocate an array containing just an
     * end marker to make code that uses this value easier */
    if (!ret)
    {
        ret = (char **)calloc(1, sizeof(char *));
    }


    return ret;
}

static void string_collection_free (char **strcol)
{
    unsigned i = 0;


    while (strcol[i])
    {
        free(strcol[i]);
        i++;
    }

    free(strcol);
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
