/*
 * Peer Statistics Module Interface
 *
 * Copyright (C) Matthew Turner 2009. All rights reserved.
 *
 * $Id: hash.h 321 2009-09-22 23:09:52Z matt $
 */

#ifndef _INCLUDED_PEERSTATS_H
#define _INCLUDED_PEERSTATS_H

#include "direntry.h"
#include "trace.h"


TRACE_DECLARE(peerstats)
#define peerstats_trace(...) TRACE(peerstats,__VA_ARGS__)
#define peerstats_trace_indent() TRACE_INDENT(peerstats)
#define peerstats_trace_dedent() TRACE_DEDENT(peerstats)


extern int peerstats_init (void);
extern void peerstats_finalise (void);

extern listing_t *peerstats_chose_alternative (listing_list_t *lis);


#endif /* _INCLUDED_PEERSTATS_H */
