/*
 * API for dealing with "direntries" - "entries in a directory". This is
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
#include <fuse/fuse_lowlevel.h>

#include "trace.h"


TRACE_DECLARE(direntry)
#define direntry_trace(...) TRACE(direntry,__VA_ARGS__)
#define direntry_trace_indent() TRACE_INDENT(direntry)
#define direntry_trace_dedent() TRACE_DEDENT(direntry)


typedef enum
{
    direntry_type_FILE,
    direntry_type_DIRECTORY
} direntry_type_t;

typedef struct _direntry_t direntry_t;
typedef struct _listing_t listing_t;

typedef struct
{
    unsigned count;
    listing_t **items;
} listing_list_t;


extern int direntry_init (void);
extern void direntry_finalise (void);

extern direntry_t *direntry_post      (CALLER_DECL direntry_t *de);
extern direntry_t *direntry_post_list (CALLER_DECL direntry_t *de);
extern void direntry_delete      (CALLER_DECL direntry_t *de);
extern void direntry_delete_list (CALLER_DECL direntry_t *de);

extern direntry_t *direntry_get_parent       (direntry_t *de);
extern direntry_t *direntry_get_first_child  (direntry_t *de);
extern direntry_t *direntry_get_next_sibling (direntry_t *de);

extern ino_t           direntry_get_inode               (direntry_t *de);
extern char *          direntry_get_path                (direntry_t *de);
extern char *          direntry_get_base_name           (direntry_t *de);
extern char *          direntry_get_hash                (direntry_t *de);
extern direntry_type_t direntry_get_type                (direntry_t *de);
extern off_t           direntry_get_size                (direntry_t *de);
extern unsigned long   direntry_get_link_count          (direntry_t *de);
extern char *          direntry_get_href                (direntry_t *de);
extern int             direntry_get_looked_for_children (direntry_t *de);
extern void            direntry_set_looked_for_children (direntry_t *de, int val);
extern int             direntry_is_root                 (direntry_t *de);
extern void            direntry_de2stat                 (direntry_t *de, struct stat *st);
extern void            direntry_still_exists            (direntry_t *de);
extern void            direntry_no_longer_exists        (direntry_t *de);


extern listing_t *listing_new (CALLER_DECL_ONLY);
extern listing_t *listing_post (CALLER_DECL listing_t *li);
extern void listing_delete (CALLER_DECL listing_t *li);

extern listing_list_t *listing_list_new (unsigned count);
extern listing_list_t *listing_list_resize (listing_list_t *lis, unsigned new_count);
extern void listing_list_delete (CALLER_DECL listing_list_t *lis);

extern char *          listing_get_name          (listing_t *li);
extern char *          listing_get_hash          (listing_t *li);
extern direntry_type_t listing_get_type          (listing_t *li);
extern off_t           listing_get_size          (listing_t *li);
extern unsigned long   listing_get_link_count    (listing_t *li);
extern char *          listing_get_href          (listing_t *li);
extern char *          listing_get_client        (listing_t *li);

extern int path_get_direntry (
    char const * const path,
    direntry_t **direntry
);
extern int direntry_get_children (
    direntry_t *de,
    direntry_t **children_out
);
void listing_attribute_add (
    listing_t * const li,
    const char *name,
    const char *value
);


/* FIXME: stubs */
extern int direntry_get_by_inode (fuse_ino_t ino, direntry_t **de);
extern int direntry_get_by_parent_and_name (
    fuse_ino_t parent,
    const char *name,
    direntry_t **de_out
);

#endif /* _INCLUDED_DIRENTRY_H */
