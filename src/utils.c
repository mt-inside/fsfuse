/*
 * Common utilities.
 *
 * Copyright (C) Matthew Turner 2008-2012. All rights reserved.
 *
 * $Id: common.h 586 2012-03-26 18:14:00Z matt $
 */

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"


static pthread_key_t thread_index_key;
static unsigned next_thread_index = 1;


static void thread_index_destroy (void *i);


int utils_init (void)
{
    int rc;


    rc = pthread_key_create(&thread_index_key, &thread_index_destroy);


    return rc;
}

void utils_finalise (void)
{
    unsigned *i = (unsigned *)pthread_getspecific(thread_index_key);
    thread_index_destroy((void *)i);

    pthread_key_delete(thread_index_key);
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


/* Is this string IPv4 address? */
int is_ip4_address (const char *s)
{
    return (strspn(s, "0123456789.") == strlen(s));
}

/* Is this string IPv6 address? */
int is_ip6_address (const char *s)
{
    return (strspn(s, "0123456789abcdefABCDEF:") == strlen(s));
}


int hash_equal (char *h1, char *h2)
{
    return !strcmp(h1, h2);
}
