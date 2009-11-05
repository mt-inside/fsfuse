/*
 * Portable lock / mutex / semaphore implementation.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include "common.h"
#include "locks.h"


struct _rw_lock_t
{
    int readers_reading;
    int writer_writing;
    pthread_mutex_t mutex;
    pthread_cond_t lock_free;
    pid_t locking_thread;
};


rw_lock_t *rw_lock_new (void)
{
    rw_lock_t *mutex = (rw_lock_t *)malloc(sizeof(*mutex));


    memset((void *)mutex, 0, sizeof(rw_lock_t));
    pthread_mutex_init(&(mutex->mutex), NULL);
    pthread_cond_init(&(mutex->lock_free), NULL);


    return mutex;
}

int rw_lock_delete (rw_lock_t *mutex)
{
    pthread_cond_destroy(&(mutex->lock_free));
    pthread_mutex_destroy(&(mutex->mutex));

    free(mutex);


    return 0;
}

int rw_lock_rlock (rw_lock_t *mutex)
{
    if (mutex->locking_thread &&
        mutex->locking_thread == syscall(SYS_gettid))
    {
        return 0;
    }

    pthread_mutex_lock(&(mutex->mutex));

    while (mutex->writer_writing)
    {
        pthread_cond_wait(&(mutex->lock_free), &(mutex->mutex));
    }
    mutex->readers_reading++;

    mutex->locking_thread = syscall(SYS_gettid);

    pthread_mutex_unlock(&(mutex->mutex));


    return 0;
}

int rw_lock_runlock (rw_lock_t *mutex)
{
    int rc;


    pthread_mutex_lock(&(mutex->mutex));

    if (!mutex->readers_reading)
    {
        rc = -1;
    }
    else
    {
        mutex->readers_reading--;
        if (!mutex->readers_reading)
        {
            pthread_cond_signal(&(mutex->lock_free));

            mutex->locking_thread = 0;
        }

        rc = 0;
    }


    pthread_mutex_unlock(&(mutex->mutex));


    return rc;
}

int rw_lock_wlock (rw_lock_t *mutex)
{
    if (mutex->locking_thread &&
        mutex->locking_thread == syscall(SYS_gettid))
    {
        return 0;
    }

    pthread_mutex_lock(&(mutex->mutex));

    while (mutex->writer_writing || mutex->readers_reading)
    {
        pthread_cond_wait(&(mutex->lock_free), &(mutex->mutex));
    }
    mutex->writer_writing++;

    mutex->locking_thread = syscall(SYS_gettid);

    pthread_mutex_unlock(&(mutex->mutex));


    return 0;
}

int rw_lock_wunlock (rw_lock_t *mutex)
{
    int rc;


    pthread_mutex_lock(&(mutex->mutex));

    if (!mutex->writer_writing)
    {
        rc = -1;
    }
    else
    {
        mutex->writer_writing = 0;
        pthread_cond_broadcast(&(mutex->lock_free));
        rc = 0;
        mutex->locking_thread = 0;
    }


    pthread_mutex_unlock(&(mutex->mutex));


    return rc;
}
