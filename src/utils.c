/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Common utilities.
 */

#include "common.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "utils.h"


static pthread_key_t thread_index_key;
static unsigned next_thread_index = 1;


static void thread_index_destroy (void *i);


/* This has to happen in the same thread as finalise. And that has to be the
 * main thread. Horrid. Use RAII instead */
int utils_init (void)
{
    int rc;


    rc = pthread_key_create(&thread_index_key, &thread_index_destroy);


    return rc;
}

void utils_finalise (void)
{
    /* Have to clean up /this/ thread's specific data, because we have to delete
     * the key here which means the destructor won't be called. Obviously this
     * is horrid */
    unsigned *i = (unsigned *)pthread_getspecific(thread_index_key);
    thread_index_destroy((void *)i);

    assert(!pthread_key_delete(thread_index_key));
}


/* There are versions of these functions glibc, but they're not thread-safe.
 * These implementations are not complete, e.g. they don't drop trailing '/'s,
 * but this isn't needed as we always deal in normalised paths.
 * These functions are called often, so should aim to be fast. */
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
        memcpy(dir, path, dir_len);
        dir[dir_len] = '\0';
    }


    return dir;
}

#ifndef _GNU_SOURCE
#define memrchr(s,c,n) strrchr(s,c)
#endif
char *fsfuse_basename (const char *path)
{
    size_t path_len = strlen(path);
    char *loc = memrchr(path, '/', path_len);
    unsigned base_len;
    char *base = NULL;


    if (!loc) return strdup(path);

    base_len = path_len - (loc - path) + 1;
    if (!base_len) return strdup("");

    base = (char *)malloc((base_len + 1) * sizeof(char));
    if (base)
    {
        memcpy(base, loc + 1, base_len);
        base[base_len] = '\0';
    }


    return base;
}

/* TODO: is there a library call for this? */
/* TODO: unit test me */
char *path_combine (const char *a, const char *b)
{
    size_t a_len = strlen(a),
           b_len = strlen(b),
           r_len = a_len + b_len;
    char *r;
    int add_delimiter = 0;


    if (a[a_len - 1] != '/') add_delimiter = 1;
    assert(b[0] != '/');

    if (add_delimiter) r_len++;
    r = malloc(r_len + 1);
    strcpy(r, a);
    if (add_delimiter) { r[a_len] = '/'; a_len++; }
    assert(a_len + b_len == r_len);
    strcpy(r + a_len, b);
    r[r_len] = '\0';

    free_const(a);
    free_const(b);


    return r;
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


/* assumes both strings are well-formed */
int compare_dotted_version (const char *ver, const char *cmp)
{
    int n1, n2;

    while (1)
    {
        n1 = strtol(ver, (char **)&ver, 0);
        n2 = strtol(cmp, (char **)&cmp, 0);

        if (n1 < n2) return -1;
        if (n1 > n2) return 1;

        if (*ver == '\0' && *cmp == '\0') return 0;

        if (*ver) ver++;
        if (*cmp) cmp++;
    }
}


int hash_equal (const char *h1, const char *h2)
{
    return !strcmp(h1, h2);
}
