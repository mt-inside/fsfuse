/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * A simple type-safe linked list in macros.
 * Easier to use, but less flexible, than queue.h
 */

#ifndef _INCLUDED_LINKED_LIST_H
#define _INCLUDED_LINKED_LIST_H

#include <stdlib.h>


#define LINKED_LIST_ENTRY_T(LINKED_LIST_TYPE_TAG, TYPE) \
struct LINKED_LIST_TYPE_TAG \
{ \
    TYPE data; \
    struct LINKED_LIST_TYPE_TAG *next; \
}

#define LINKED_LIST_T(LINKED_LIST_TYPE_TAG) struct LINKED_LIST_TYPE_TAG *

#define LINKED_LIST_INIT NULL

#define LINKED_LIST_ADD(HEAD, DATA) \
typeof(HEAD) temp = HEAD; \
HEAD = malloc( sizeof(*HEAD) ); \
HEAD->data = DATA; \
HEAD->next = temp

#define LINKED_LIST_FOREACH(HEAD, CUR) \
typeof(HEAD) _item; \
typeof(_item->data) CUR; \
for( _item = HEAD; ( _item && ((CUR = _item->data), 1) ); _item = _item->next )

#define LINKED_LIST_DELETE(HEAD, ITEM_DELETE_FN) \
typeof(HEAD) _item, _next; \
for( _item = HEAD; ( _item && ((_next = _item->next), 1) ); _item = _next ) \
{ \
    ITEM_DELETE_FN( _item->data ); \
    free( _item ); \
}

#endif /* _INCLUDED_LINKED_LIST_H */
