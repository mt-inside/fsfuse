/*
 * Portable lock / mutex / semaphore API.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#include <pthread.h>


typedef struct
{
  int readers_reading;
  int writer_writing;
  pthread_mutex_t mutex;
  pthread_cond_t lock_free;
} rw_lock_t;


int rw_lock_init   (rw_lock_t *mutex);
int rw_lock_destroy(rw_lock_t *mutex);
int rw_lock_rlock  (rw_lock_t *mutex);
int rw_lock_runlock(rw_lock_t *mutex);
int rw_lock_wlock  (rw_lock_t *mutex);
int rw_lock_wunlock(rw_lock_t *mutex);
