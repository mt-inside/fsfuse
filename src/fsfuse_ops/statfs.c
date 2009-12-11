/*
 * statfs() implementation.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#include <fuse.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include "common.h"
#include "fetcher.h"
#include "fsfuse_ops/fsfuse_ops.h"
#include "parser.h"


static void stats_general_parse (struct statvfs *stfs, xmlNodeSetPtr nodes);


int fsfuse_statfs (const char *path, struct statvfs *stfs)
{
    xmlParserCtxtPtr  parser;
    xmlXPathObjectPtr xpathObj;
    xmlDocPtr         doc;
    int rc = -EIO, fetcher_rc;


    method_trace("fsfuse_statfs(path==%s)\n", path);
    method_trace_indent();

    memset(stfs, 0, sizeof(struct statvfs));
    stfs->f_bsize   = FSFUSE_BLKSIZE;
    stfs->f_frsize  = FSFUSE_BLKSIZE;        /* Ignored by fuse */
    stfs->f_flag    = ST_RDONLY | ST_NOSUID; /* Ignored by fuse */
    stfs->f_namemax = ULONG_MAX;

    /* make a new parser for this thread */
    parser = parser_new();

    /* fetch the stats page and feed it into the parser */
    fetcher_rc = fetcher_fetch_stats((curl_write_callback)&parser_consumer,
                                     (void *)parser);

    if (fetcher_rc == 0)
    {
        /* The fetcher has returned, so that's all the document.
         * Indicate to the parser that that's it */
        doc = parser_done(parser);

        xpathObj = parser_xhtml_xpath(doc, "//xhtml:div[@id='general']/xhtml:span[@id]");
        if (xpathObj->type == XPATH_NODESET)
        {
            stats_general_parse(stfs, xpathObj->nodesetval);
            rc = 0;
        }

        xmlXPathFreeObject(xpathObj);
        xmlFreeDoc(doc);
    }

    parser_delete(parser);

    method_trace_dedent();


    return rc;
}

static void stats_general_parse (struct statvfs *stfs, xmlNodeSetPtr nodes)
{
    xmlNodePtr curNod = NULL;
    int size, i;
    char *id, *value;


    size = (nodes) ? nodes->nodeNr : 0;

    parser_trace("stats_general_parse(): enumerating %d nodes\n", size);
    trace_indent();

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

                stfs->f_files = atoll(value);
                parser_trace("file-count: %llu\n", stfs->f_files);

                free(value);
            }
            else if (!strcmp(id, "total-size"))
            {
                value = (char *)xmlGetProp(curNod, BAD_CAST "value");

                stfs->f_blocks = atoll(value) / stfs->f_bsize;
                parser_trace("total-size: %llu %lu byte blocks\n",
                     stfs->f_blocks, stfs->f_bsize);

                free(value);
            }
            free(id);
        }
    }

    trace_dedent();
}
