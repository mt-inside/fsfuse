/*
 * Useful common utility functions.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#include <stdlib.h>
#include <string.h>

#include "common.h"


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
