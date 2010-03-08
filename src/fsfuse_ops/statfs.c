/*
 * statfs() implementation.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#include <fuse/fuse_lowlevel.h>
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


static void stats_general_parse (struct statvfs *stvfs, xmlNodeSetPtr nodes);


void fsfuse_statfs (fuse_req_t req, fuse_ino_t ino)
{
    xmlParserCtxtPtr  parser;
    xmlXPathObjectPtr xpathObj;
    xmlDocPtr         doc;
    struct statvfs stvfs;
    int rc;


    NOT_USED(ino);

    method_trace("fsfuse_statfs(ino %lu)\n", ino);
    method_trace_indent();

    memset(&stvfs, 0, sizeof(struct statvfs));
    stvfs.f_bsize   = FSFUSE_BLKSIZE;
    stvfs.f_frsize  = FSFUSE_BLKSIZE;        /* Ignored by fuse */
    stvfs.f_flag    = ST_RDONLY | ST_NOSUID; /* Ignored by fuse */
    stvfs.f_namemax = ULONG_MAX;

    /* make a new parser for this thread */
    parser = parser_new();

    /* fetch the stats page and feed it into the parser */
    rc = fetcher_fetch_stats((curl_write_callback)&parser_consumer,
                             (void *)parser);

    if (!rc)
    {
        /* The fetcher has returned, so that's all the document.
         * Indicate to the parser that that's it */
        doc = parser_done(parser);

        xpathObj = parser_xhtml_xpath(doc, "//xhtml:div[@id='general']/xhtml:span[@id]");
        if (xpathObj->type == XPATH_NODESET)
        {
            stats_general_parse(&stvfs, xpathObj->nodesetval);
            rc = 0;
        }

        xmlXPathFreeObject(xpathObj);
        xmlFreeDoc(doc);
    }

    parser_delete(parser);

    method_trace_dedent();


    if (!rc)
    {
        assert(!fuse_reply_statfs(req, &stvfs));
    }
    else
    {
        assert(!fuse_reply_err(req, rc));
    }
}

static void stats_general_parse (struct statvfs *stvfs, xmlNodeSetPtr nodes)
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

                stvfs->f_files = atoll(value);
                parser_trace("file-count: %llu\n", stvfs->f_files);

                free(value);
            }
            else if (!strcmp(id, "total-size"))
            {
                value = (char *)xmlGetProp(curNod, BAD_CAST "value");

                stvfs->f_blocks = atoll(value) / stvfs->f_bsize;
                parser_trace("total-size: %llu %lu byte blocks\n",
                     stvfs->f_blocks, stvfs->f_bsize);

                free(value);
            }
            free(id);
        }
    }

    trace_dedent();
}
