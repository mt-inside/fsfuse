/*
 * API for dealing with "direntrys" - "entries in a directory". This is
 * basically our internal file system tree implementation.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#ifndef _INCLUDED_DIRENTRY_H
#define _INCLUDED_DIRENTRY_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "trace.h"


TRACE_DECLARE(direntry)

typedef enum
{
    direntry_type_FILE,
    direntry_type_DIRECTORY
} direntry_type_t;

typedef struct _direntry_t direntry_t;


extern int direntry_init (void);
extern void direntry_finalise (void);

extern direntry_t *direntry_new (CALLER_DECL_ONLY);
extern void direntry_post        (CALLER_DECL direntry_t *de);
extern void direntry_delete      (CALLER_DECL direntry_t *de);
extern void direntry_delete_list (direntry_t *de);

extern direntry_t *direntry_get_parent       (direntry_t *de);
extern direntry_t *direntry_get_first_child  (direntry_t *de);
extern direntry_t *direntry_get_next_sibling (direntry_t *de);

extern char *          direntry_get_path       (direntry_t *de);
extern char *          direntry_get_base_name  (direntry_t *de);
extern char *          direntry_get_hash       (direntry_t *de);
extern direntry_type_t direntry_get_type       (direntry_t *de);
extern off_t           direntry_get_size       (direntry_t *de);
extern unsigned long   direntry_get_link_count (direntry_t *de);
extern char *          direntry_get_href       (direntry_t *de);
extern int             direntry_got_children   (direntry_t *de);
extern int             direntry_is_root        (direntry_t *de);

extern int direntry_get               (const char * const path, direntry_t **de);
extern int direntry_get_with_children (const char * const path, direntry_t **de);
extern int direntry_get_children      (direntry_t *de, direntry_t **de_out);

extern int direntry_de2stat (struct stat *st, direntry_t *de);
extern char *fsfuse_dirname (const char *path);
extern char *fsfuse_basename (const char *path);

#endif /* _INCLUDED_DIRENTRY_H */
