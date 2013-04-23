
/* TODO: paring (or at least libxml2 interaction should be in parser.h */

#include "common.h"

#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <string.h>

#include "config_loader.h"
#include "config_internal.h"


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

int config_loader_tryread_from_file (const char *config_file_path, config_data_t **data_out)
{
    config_data_t *data = malloc(sizeof(*data));
    config_xml_info_item_t *item;
    char *val, **val_array;
    xmlDocPtr doc;
    xmlXPathContextPtr xpathCtxt;
    int rc = 0;


    /* Read the config file */
    doc = xmlParseFile(config_file_path);

    if (doc)
    {
        rc = 1;
        xpathCtxt = xmlXPathNewContext(doc);

        for (item = config_xml_info; item->offset; item++)
        {
            switch (item->type)
            {
                case config_item_type_STRING:
                    val = xpath_get(item->xpath, xpathCtxt);

                    if (val)
                    {
                        *((char **)(data + item->offset)) = strdup(val);
                        *((int *)(data + item->offset + sizeof(char *))) = 1;
                    }

                    break;

                case config_item_type_INTEGER:
                    val = xpath_get(item->xpath, xpathCtxt);

                    if (val)
                    {
                        *((int *)(data + item->offset)) = strtoul(val, NULL, 0);
                        *((int *)(data + item->offset + sizeof(int))) = 1;
                    }

                    break;

                case config_item_type_FLOAT:
                    val = xpath_get(item->xpath, xpathCtxt);

                    if (val)
                    {
                        *((double *)(data + item->offset)) = strtod(val, NULL);
                        *((int *)(data + item->offset + sizeof(double))) = 1;
                    }

                    break;

                case config_item_type_STRING_COLLECTION:
                    val_array = xpath_get_array(item->xpath, xpathCtxt);

                    if (val_array)
                    {
                        *((char ***)(data + item->offset)) = val_array;
                        *((int *)(data + item->offset + sizeof(char **))) = 1;
                    }

                    break;
            }
        }

        xmlXPathFreeContext(xpathCtxt);
        xmlFreeDoc(doc);
    }
    else
    {
        trace_warn("Unable to read / parse config file %s\n", config_file_path);
    }

    free_const(config_file_path);


    if (rc == 1) *data_out = data;

    return rc;
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

void config_loader_items_free (config_data_t *data)
{
    config_xml_info_item_t *item;


    for (item = config_xml_info; item->offset; item++)
    {
        switch (item->type)
        {
            case config_item_type_STRING:
                free(*((char **)(data + item->offset)));
                break;

            case config_item_type_INTEGER:
            case config_item_type_FLOAT:
                break;

            case config_item_type_STRING_COLLECTION:
                string_collection_free(*((char ***)(data + item->offset)));
                break;
        }
    }
}
