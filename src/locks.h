/*
 * Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
 *
 * Portable lock / mutex / semaphore API.
 */

#ifndef _INCLUDED_LOCKS_H
#define _INCLUDED_LOCKS_H

#include <pthread.h>


typedef struct _rw_lock_t rw_lock_t;


extern rw_lock_t *rw_lock_new     (void);
extern int        rw_lock_delete  (rw_lock_t *mutex);
extern int        rw_lock_rlock   (rw_lock_t *mutex);
extern int        rw_lock_runlock (rw_lock_t *mutex);
extern int        rw_lock_wlock   (rw_lock_t *mutex);
extern int        rw_lock_wunlock (rw_lock_t *mutex);

#endif /* _INCLUDED_LOCKS_H */
