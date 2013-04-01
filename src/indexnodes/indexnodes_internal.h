/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * Internal declarations of the indexnodes manager class.
 */

#ifndef _INCLUDED_INDEXNODES_INTERNAL_H
#define _INCLUDED_INDEXNODES_INTERNAL_H

typedef void (*new_indexnode_event_t) (
    const void *ctxt,
    const char * const host,
    const char * const port,
    const char * const fs2protocol,
    const char * const id
);

#endif /* _INCLUDED_INDEXNODES_INTERNAL_H */
