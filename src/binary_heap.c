/*
 * Copyright (C) 2008-2013 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Binary (min)-heap data sructure. Mostly meant for the Pri-Q ADT.
 */
#include "common.h"

#include <stdlib.h>

#include "binary_heap.h"

#include "kvp.h"


struct _binary_heap_t
{
    size_t allocated;
    size_t used;
    kvp_t **array;
};


binary_heap_t *binary_heap_new( void )
{
    binary_heap_t *heap = calloc( 1, sizeof(*heap) );

    return heap;
}

void binary_heap_delete( binary_heap_t *heap )
{
    assert( heap->used == 0 );

    free( heap->array );
    free( heap );
}

static void ensure_space( binary_heap_t *heap, size_t space )
{
    if( heap->allocated < space )
    {
        size_t new_space = MAX( heap->allocated * 2, space );
        heap->array = realloc( heap->array, new_space * sizeof(*(heap->array)) );
        heap->allocated = new_space;
    }
}

static size_t parent( int node ) { return ( node - 1 ) / 2; }
static size_t left_child( int node ) { return ( 2 * node ) + 1; }
static size_t right_child( int node ) { return ( 2 * node ) + 2; }

static void move_up( binary_heap_t *heap, size_t target_pos )
{
    while( target_pos > 0 )
    {
        kvp_t *target_node = heap->array[ target_pos ];
        size_t parent_pos = parent( target_pos );
        kvp_t *parent_node = heap->array[ parent_pos ];

        if( kvp_key( target_node ) >= kvp_key( parent_node ) ) break;

        SWAP( heap->array[ target_pos ], heap->array[ parent_pos ] );
        target_pos = parent_pos;
    }
}

void binary_heap_add( binary_heap_t *heap, int key, void *value )
{
    ensure_space( heap, heap->used + 1 );
    heap->array[ heap->used ] = kvp_new( key, value );

    move_up( heap, heap->used );

    heap->used++;
}

void move_down( binary_heap_t *heap, size_t target_pos )
{
    while( target_pos < heap->used / 2 )
    {
        kvp_t *target_node = heap->array[ target_pos ];

        size_t child_left_pos = left_child( target_pos );
        kvp_t *child_left_node = heap->array[ child_left_pos ];
        size_t child_right_pos = right_child( target_pos );
        kvp_t *child_right_node = heap->array[ child_right_pos ];
        size_t child_pos;
        kvp_t *child_node;

        if( child_left_pos > heap->used - 1 ||
            kvp_key( child_left_node ) > kvp_key( child_right_node ) )
        {
            child_pos = child_right_pos;
            child_node = child_right_node;
        }
        else
        {
            child_pos = child_left_pos;
            child_node = child_left_node;
        }

        if( kvp_key( target_node ) <= kvp_key( child_node ) ) break;

        SWAP( heap->array[ target_pos ], heap->array[ child_pos ] );
        target_pos = child_pos;
    }
}

int binary_heap_trypop( binary_heap_t *heap, int *key, void **value )
{
    int rc = 0;

    if( heap->used > 0 )
    {
        kvp_t *target_node = heap->array[ 0 ];
        *key = kvp_key( target_node );
        *value = kvp_value( target_node );
        rc = 1;

        /* TODO: think about reducing the allocated size back down when stuffs are taken
         * out */
        heap->used--;
        if( heap->used > 0 )
        {
            kvp_t *last_node = heap->array[ heap->used ];
            heap->array[ 0 ] = last_node;
            move_down( heap, 0 );
        }
    }

    return rc;
}
