/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * Class to manage the set of statically-configured indexnodes.
 */

#ifndef _INCLUDED_INDEXNODES_INDEXNODES_STATICS_MANAGER_H
#define _INCLUDED_INDEXNODES_INDEXNODES_STATICS_MANAGER_H

#include "common.h"

#include "indexnodes_internal.h"


typedef struct _indexnodes_statics_manager_t indexnodes_statics_manager_t;


extern indexnodes_statics_manager_t *indexnodes_statics_manager_new(
    new_indexnode_event_t cb,
    void *ctxt
);

extern void indexnodes_statics_manager_delete(
    indexnodes_statics_manager_t *mgr
);

#endif /* _INCLUDED_INDEXNODES_INDEXNODES_STATICS_MANAGER_H */
