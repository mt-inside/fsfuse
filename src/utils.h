/*
 * Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
 *
 * Common utilities header file.
 */

#ifndef _INCLUDED_UTILS_H
#define _INCLUDED_UTILS_H

extern int utils_init (void);
extern void utils_finalise (void);

extern char *fsfuse_dirname (const char *path);
extern char *fsfuse_basename (const char *path);
extern char *path_combine (const char *a, const char *b);

extern unsigned fsfuse_get_thread_index (void);

extern int compare_dotted_version (const char *ver, const char *cmp);

extern int hash_equal (char *h1, char *h2);

#endif /* _INCLUDED_UTILS_H */
