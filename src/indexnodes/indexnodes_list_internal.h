/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * A list of indexnodes.
 * INTERNAL TO INDEXNODE CLASSES; all public interaction should be via the
 * iterator class.
 *
 * Lists, like their iterators, are meant to be short-lived. They're not ref
 * counted because you're not meant to hang on to them for long - just get a new
 * one. If you do you'll just find they're full of dead indexnodes and don't
 * contain new ones.
 *
 * Lists mark their indexnodes on add() and delete() them when they're deleted
 * themselves.
 *
 * Lists are not thread-safe; again just get a new one.
 */

#ifndef _INCLUDED_INDEXNODES_LIST_INTERNAL_H
#define _INCLUDED_INDEXNODES_LIST_INTERNAL_H

#include "queue.h"

#include "indexnode.h"
#include "indexnodes.h"


typedef struct _item_t
{
    indexnode_t *in;
    TAILQ_ENTRY(_item_t) next;
} item_t;

struct _indexnodes_list_t
{
    TAILQ_HEAD(,_item_t) list;
};


extern indexnodes_list_t *indexnodes_list_new (void);
extern void indexnodes_list_add (indexnodes_list_t *ins, indexnode_t *in);
extern indexnode_t *indexnodes_list_find (CALLER_DECL indexnodes_list_t *ins, const char * const id);
extern indexnodes_list_t *indexnodes_list_copy (CALLER_DECL indexnodes_list_t *ins);
extern indexnodes_list_t *indexnodes_list_remove_expired (CALLER_DECL indexnodes_list_t *orig);

#endif /* _INCLUDED_INDEXNODES_LIST_INTERNAL_H */
