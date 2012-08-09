/*
 * Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
 *
 * API for dealing with "listings", which are descriptions of all the nodes in a
 * directory.
 */

#ifndef _INCLUDED_LISTING_H
#define _INCLUDED_LISTING_H

#include <sys/types.h>
#include <sys/stat.h>

#include "trace.h"


TRACE_DECLARE(listing)
#define listing_trace(...) TRACE(listing,__VA_ARGS__)
#define listing_trace_indent() TRACE_INDENT(listing)
#define listing_trace_dedent() TRACE_DEDENT(listing)


typedef struct _listing_t listing_t;
typedef struct _listing_list_t listing_list_t;


typedef enum
{
    listing_type_FILE,
    listing_type_DIRECTORY
} listing_type_t;


extern listing_t *listing_new (CALLER_DECL_ONLY);
extern listing_t *listing_post (CALLER_DECL listing_t *li);
extern void listing_delete (CALLER_DECL listing_t *li);

void listing_attribute_add (
    listing_t * const li,
    const char *name,
    const char *value
);

extern int listing_equal (listing_t *li, listing_t *other);

extern indexnode_t *   listing_get_indexnode     (listing_t *li);
extern char *          listing_get_name          (listing_t *li);
extern char *          listing_get_hash          (listing_t *li);
extern listing_type_t  listing_get_type          (listing_t *li);
extern off_t           listing_get_size          (listing_t *li);
extern unsigned long   listing_get_link_count    (listing_t *li);
extern char *          listing_get_href          (listing_t *li);
extern char *          listing_get_client        (listing_t *li);
extern void            listing_li2stat           (listing_t *li,
                                                  struct stat *st);


extern listing_list_t *listing_list_new (unsigned count);
extern listing_list_t *listing_list_resize (listing_list_t *lis, unsigned new_count);
extern void listing_list_delete (CALLER_DECL listing_list_t *lis);

extern unsigned listing_list_get_count (listing_list_t *lis);
extern void listing_list_set_item (listing_list_t *lis, unsigned item, listing_t *li);
extern listing_t *listing_list_get_item (listing_list_t *lis, unsigned item);

#endif /* _INCLUDED_LISTING_H */
