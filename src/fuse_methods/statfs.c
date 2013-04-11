/*
 * Copyright (C) 2008-2013 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * statfs() implementation.
 */

#include "common.h"

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include "fuse_methods.h"

#include "fetcher.h"
#include "indexnodes.h"
#include "parser.h"


static void stats_general_parse (struct statvfs *stvfs, xmlNodeSetPtr nodes);


void fsfuse_statfs (fuse_req_t req, fuse_ino_t ino)
{
    xmlParserCtxtPtr  parser;
    xmlXPathObjectPtr xpathObj;
    xmlDocPtr         doc;
    indexnodes_t *ins = ((fsfuse_ctxt_t *)fuse_req_userdata(req))->indexnodes;
    struct statvfs stvfs;
    int rc = 1;
    const char *url;
    indexnodes_list_t *list;
    indexnodes_iterator_t *iter;
    indexnode_t *in;


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
    /* TODO: this is so amazinlgy wrong. fetcher_f_s uses parser many times,
     * thata has to stop. */
    /* TODO: it's awful that the fetcher and this file share the state of the
     * parser, that should be wrapped up in an object */
    /* TODO: this file should not be concerned with libxml or any parsing. that
     * should sit somwhere else. How about a ctor that gets a new "i eat stats
     * results and return a total" object? */
    /* TODO: the parser_consumer callback shouldn't be in another file */
    list = indexnodes_get(CALLER_INFO ins);
    for (iter = indexnodes_iterator_begin(list);
         !indexnodes_iterator_end(iter);
         iter = indexnodes_iterator_next(iter))
    {
        in = indexnodes_iterator_current(iter);
        url = indexnode_make_url(in, "stats", "");

        rc = fetcher_fetch_internal(
            url,
            NULL,
            (curl_write_callback)&parser_consumer,
            (void *)parser
        );

        indexnode_delete(CALLER_INFO in);
        free_const(url);
    }
    indexnodes_iterator_delete(iter);
    indexnodes_list_delete(list);

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
