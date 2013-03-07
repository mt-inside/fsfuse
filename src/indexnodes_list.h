/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * A list of indexnodes. You can do very little with this except construct an
 * iterator over it.
 *
 * Lists, like their iterators, are meant to be short-lived. They're not ref
 * counted because you're not meant to hang on to them for long - just get a new
 * one. If you do you'll just find they're full of dead indexnodes and don't
 * contain new ones.
 *
 * Lists mark their indexnodes and delete() them when they're deleted
 * themselves.
 *
 * Lists are not thread-safe; again just get a new one.
 */

#ifndef _INCLUDED_INDEXNODES_LIST_H
#define _INCLUDED_INDEXNODES_LIST_H

typedef struct _indexnodes_list_t indexnodes_list_t;


extern void indexnodes_list_delete( indexnodes_list_t *ins );

#endif /* _INCLUDED_INDEXNODES_LIST_H */
