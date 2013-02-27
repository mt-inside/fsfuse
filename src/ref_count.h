/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * Reference-counting macros.
 *
 * TODO: nonono.
 * There should be a ref_count object (opaque) that de,in etc have one of and
 * call to get ref-counting serives. That also decouples its locking from any
 * other locks the object has
 */

#ifndef _INCLUDED_REF_COUNT_H
#define _INCLUDED_REF_COUNT_H

#include <assert.h>

#define REF_COUNT_FIELDS                       \
    pthread_mutex_t lock;                      \
    unsigned ref_count

#define REF_COUNT_INIT(this)                   \
    pthread_mutex_init( &(this)->lock, NULL ); \
    (this)->ref_count = 1

#define REF_COUNT_INC(this)                    \
    pthread_mutex_lock( &(this)->lock );       \
    assert( (this)->ref_count );               \
    ++(this)->ref_count;                       \
    pthread_mutex_unlock( &(this)->lock )

#define REF_COUNT_DEC(this)                    \
    pthread_mutex_lock( &(this)->lock );       \
    assert( (this)->ref_count );               \
    refc = --(this)->ref_count;                \
    pthread_mutex_unlock( &(this)->lock );

#define REF_COUNT_TEARDOWN(this)               \
    pthread_mutex_destroy( &(this)->lock )

#endif /* _INCLUDED_REF_COUNT_H */
