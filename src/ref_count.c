/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * Reference-counting class.
 *
 */

#include "common.h"

#include <stdlib.h>

#include "ref_count.h"


struct _ref_count_t
{
    pthread_mutex_t lock;
    unsigned ref_count;
};

ref_count_t *ref_count_new( )
{
    ref_count_t *refc = calloc( sizeof(struct _ref_count_t), 1 );


    pthread_mutex_init( &refc->lock, NULL );
    refc->ref_count = 1;


    return refc;
}

void ref_count_delete( ref_count_t *refc )
{
    pthread_mutex_destroy( &refc->lock );

    free( refc );
}

unsigned ref_count_inc( ref_count_t *refc /* logger, maybe null */ )
{
    pthread_mutex_lock( &refc->lock );
    assert( refc->ref_count );
    unsigned count = ++refc->ref_count;
    pthread_mutex_unlock( &refc->lock );

    return count;
}

unsigned ref_count_dec( ref_count_t *refc )
{
    pthread_mutex_lock( &refc->lock );
    assert( refc->ref_count );
    unsigned count = --refc->ref_count;
    pthread_mutex_unlock( &refc->lock );

    return count;
}
