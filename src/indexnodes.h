/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * Class to manage the collection of known indexnodes.
 */

#ifndef _INCLUDED_INDEXNODES_H
#define _INCLUDED_INDEXNODES_H

#include "indexnodes_iterator.h"

typedef struct _indexnodes_t indexnodes_t;


extern indexnodes_t *indexnodes_new (void);
extern void indexnodes_delete (indexnodes_t *ins);

extern indexnodes_list_t *indexnodes_get (indexnodes_t *ins);

#endif /* _INCLUDED_INDEXNODES_H */
