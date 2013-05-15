/*
 * Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
 *
 * Binary (min)-heap data sructure. Mostly meant for the Pri-Q ADT.
 */

#ifndef _INCLUDED_BINARY_HEAP_H
#define _INCLUDED_BINARY_HEAP_H

#include "common.h"


typedef struct _binary_heap_t binary_heap_t;


extern binary_heap_t *binary_heap_new( void );
extern void binary_heap_delete( binary_heap_t *heap );

extern void binary_heap_add( binary_heap_t *heap, int key, void *value );
extern int binary_heap_trypop( binary_heap_t *heap, int *key, void **value );

#endif /* _INCLUDED_BINARY_HEAP_H */
