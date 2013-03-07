/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * Indexnodes list iterator class.
 */

#ifndef _INCLUDED_INDEXNODES_ITERATOR_H
#define _INCLUDED_INDEXNODES_ITERATOR_H

#include "queue.h"

#include "indexnode.h"
#include "indexnodes_list.h"


typedef struct _indexnodes_iterator_t indexnodes_iterator_t;

extern indexnodes_iterator_t *indexnodes_iterator_begin( indexnodes_list_t *ins );
extern indexnode_t *indexnodes_iterator_current( indexnodes_iterator_t *iter );
extern indexnodes_iterator_t *indexnodes_iterator_next( indexnodes_iterator_t *iter_old );
extern int indexnodes_iterator_end( indexnodes_iterator_t *iter );
extern void indexnodes_iterator_delete( indexnodes_iterator_t *iter );

#endif /* _INCLUDED_INDEXNODES_ITERATOR_H */
