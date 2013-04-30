/*
 * Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
 *
 * Peer Statistics Module Interface
 */

#ifndef _INCLUDED_PEERSTATS_H
#define _INCLUDED_PEERSTATS_H

#include "listing.h"
#include "listing_list.h"
#include "trace.h"


TRACE_DECLARE(peerstats)
#define peerstats_trace(...) TRACE(peerstats,__VA_ARGS__)
#define peerstats_trace_indent() TRACE_INDENT(peerstats)
#define peerstats_trace_dedent() TRACE_DEDENT(peerstats)


extern listing_t *peerstats_chose_alternative (listing_list_t *lis);

#endif /* _INCLUDED_PEERSTATS_H */
