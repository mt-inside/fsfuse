/*
 * Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
 *
 * A list of indexnodes.
 * Lists are not ref counted, but the indexnodes in them are.
 * Either way, when you are given a list, you own both the list and the items,
 * and must delete() both.
 * Lists are not thread-safe; if you want to use one from multiple threads you
 * should do the locking yourself.
 */

#ifndef _INCLUDED_INDEXNODES_LIST_H
#define _INCLUDED_INDEXNODES_LIST_H

#include "queue.h"

#include "indexnode.h"


typedef struct _indexnodes_list_item_t
{
    indexnode_t *in;
    TAILQ_ENTRY(_indexnodes_list_item_t) next;
} indexnodes_list_item_t;

typedef struct _indexnodes_list_t
{
    TAILQ_HEAD(,_indexnodes_list_item_t) list;
} indexnodes_list_t;


extern indexnodes_list_t *indexnodes_list_new (void);
extern void indexnodes_list_add (indexnodes_list_t *ins, indexnode_t *in);
extern indexnode_t *indexnodes_list_find (indexnodes_list_t *ins, const char * const id);
extern indexnodes_list_t *indexnodes_list_copy (indexnodes_list_t *ins);
extern indexnodes_list_t *indexnodes_list_remove_expired (indexnodes_list_t *orig);
extern void indexnodes_list_delete (indexnodes_list_t *ins);

#endif /* _INCLUDED_INDEXNODES_LIST_H */
