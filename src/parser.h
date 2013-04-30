/*
 * Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
 *
 * API for the interface to the library used to parse e.g. metadata files.
 */

#ifndef _INCLUDED_PARSER_H
#define _INCLUDED_PARSER_H

#include "common.h"

#include "nativefs.h"


TRACE_DECLARE(parser)
#define parser_trace(...) TRACE(parser,__VA_ARGS__)
#define parser_trace_indent() TRACE_INDENT(parser)
#define parser_trace_dedent() TRACE_DEDENT(parser)


typedef struct _parser_t parser_t;


extern int parser_init (void);
extern void parser_finalise (void);

extern parser_t *parser_new (void);
extern void parser_delete (parser_t *parser);

extern int parser_consumer (void *ctxt, void *data, size_t len);

extern int parser_tryget_listing(
    parser_t *parser,
    nativefs_entry_found_cb_t entry_cb,
    void *entry_ctxt
);
extern int parser_tryget_stats(
    parser_t *parser,
    unsigned long *files,
    unsigned long *bytes
);

#endif /* _INCLUDED_PARSER_H */
