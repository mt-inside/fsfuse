/*
 * Useful common utility functions.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "common.h"


static pthread_key_t thread_index_key;
static unsigned next_thread_index = 1;


static void thread_index_destroy (void *i);


int common_init (void)
{
    int rc;


    rc = pthread_key_create(&thread_index_key, &thread_index_destroy);


    return rc;
}

void common_finalise (void)
{
    pthread_key_delete(thread_index_key);
}


/* There are versions of these functions glibc, but they're not thread-safe */
char *fsfuse_dirname (const char *path)
{
    char *loc = strrchr(path, '/');
    unsigned dir_len;
    char *dir = NULL;


    if (!loc) return strdup("");

    dir_len = loc - path;
    if (!dir_len) return strdup("/");

    dir = (char *)malloc((dir_len + 1) * sizeof(char));
    if (dir)
    {
        strncpy(dir, path, dir_len);
        dir[dir_len] = '\0';
    }


    return dir;
}

char *fsfuse_basename (const char *path)
{
    char *loc = strrchr(path, '/');
    unsigned base_len;
    char *base = NULL;


    if (!loc) return strdup(path);

    base_len = strlen(path) - (loc - path) + 1;
    if (!base_len) return strdup("");

    base = (char *)malloc((base_len + 1) * sizeof(char));
    if (base)
    {
        strncpy(base, loc + 1, base_len);
        base[base_len] = '\0';
    }


    return base;
}


unsigned fsfuse_get_thread_index (void)
{
    unsigned *i = (unsigned *)pthread_getspecific(thread_index_key);


    if (!i)
    {
        i = (unsigned *)malloc(sizeof(unsigned));
        *i = next_thread_index++;
        pthread_setspecific(thread_index_key, (void *)i);
    }


    return *i;
}

static void thread_index_destroy (void *i)
{
    free((unsigned *)i);
}
