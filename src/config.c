/*
 * XML file based configuration component with a simple API.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#include "common.h"
#include "config.h"

#include <assert.h>

#include <libxml/parser.h>
#include <libxml/xpath.h>


#define CONFIG_FILE_PATH "fsfuse.conf"


/* the config */
char *config_alias;
int   config_timeout_chunk;
int   config_timeout_cache;
int   config_indexnode_autodetect;
char *config_indexnode_host;
char *config_indexnode_port;
int   config_attr_mode_file;
int   config_attr_mode_dir;
int   config_attr_id_uid;
int   config_attr_id_gid;
int   config_proc_fg;
int   config_proc_singlethread;
int   config_proc_debug;
int   config_option_cache;
int   config_option_progress;


static char *xpath_get (const char *xpath, xmlXPathContextPtr xpathCtxt);


int config_init (void)
{
    /* do nothing */

    return 0;
}

void config_finalise (void)
{
    /* do nothing */
}


int config_read (void)
{
    xmlDocPtr doc;
    xmlXPathContextPtr xpathCtxt;


    /* Read the config file */
    doc = xmlParseFile(CONFIG_FILE_PATH);
    assert(doc);


    xpathCtxt = xmlXPathNewContext(doc);

    config_alias                =      xpath_get("/config/alias/text()",                     xpathCtxt);
    config_timeout_chunk        = atoi(xpath_get("/config/timeouts/chunk/text()",            xpathCtxt));
    config_timeout_cache        = atoi(xpath_get("/config/timeouts/cache/text()",            xpathCtxt));
    config_indexnode_autodetect = atoi(xpath_get("/config/indexnode/autodetect/text()",      xpathCtxt));
    config_indexnode_host       =      xpath_get("/config/indexnode/host/text()",            xpathCtxt);
    config_indexnode_port       =      xpath_get("/config/indexnode/port/text()",            xpathCtxt);
    config_attr_mode_file       = atoi(xpath_get("/config/node-attrs/mode/file/text()",      xpathCtxt));
    config_attr_mode_dir        = atoi(xpath_get("/config/node-attrs/mode/directory/text()", xpathCtxt));
    config_attr_id_uid          = atoi(xpath_get("/config/node-attrs/id/uid/text()",         xpathCtxt));
    config_attr_id_gid          = atoi(xpath_get("/config/node-attrs/id/gid/text()",         xpathCtxt));
    config_proc_fg              = atoi(xpath_get("/config/process/foreground/text()",        xpathCtxt));
    config_proc_singlethread    = atoi(xpath_get("/config/process/single-thread/text()",     xpathCtxt));
    config_proc_debug           = atoi(xpath_get("/config/process/debug/text()",             xpathCtxt));
    config_option_cache         = atoi(xpath_get("/config/options/cache/text()",             xpathCtxt));
    config_option_progress      = atoi(xpath_get("/config/options/progress/text()",          xpathCtxt));


    xmlXPathFreeContext(xpathCtxt);
    xmlFreeDoc(doc);


    return 0;
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
