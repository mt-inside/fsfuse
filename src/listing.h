/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * Listing class.
 * A listing describes an entry in a directory. It describes only the attributes
 * of that node and knows nothing of its position in the tree.
 */

#ifndef _INCLUDED_LISTING_H
#define _INCLUDED_LISTING_H

#include "common.h"

#include <sys/types.h>
#include <sys/stat.h>

#include "trace.h"


TRACE_DECLARE(listing)
#define listing_trace(...) TRACE(listing,__VA_ARGS__)
#define listing_trace_indent() TRACE_INDENT(listing)
#define listing_trace_dedent() TRACE_DEDENT(listing)


typedef struct _listing_t listing_t;


typedef enum
{
    listing_type_FILE,
    listing_type_DIRECTORY
} listing_type_t;


#include "indexnode.h"

extern listing_t *listing_new (
    CALLER_DECL
    indexnode_t *in,
    const char *hash,
    const char *name,
    const char *type,
    off_t size,
    unsigned long link_count,
    const char *href,
    const char *client
);
extern listing_t *listing_post (CALLER_DECL listing_t *li);
extern void listing_delete (CALLER_DECL listing_t *li);

extern int listing_equal (listing_t *li, listing_t *other);

extern char *          listing_get_name          (listing_t *li);
extern char *          listing_get_hash          (listing_t *li);
extern listing_type_t  listing_get_type          (listing_t *li);
extern off_t           listing_get_size          (listing_t *li);
extern unsigned long   listing_get_link_count    (listing_t *li);
extern char *          listing_get_href          (listing_t *li);
extern char *          listing_get_client        (listing_t *li);
extern void            listing_li2stat           (listing_t *li,
                                                  struct stat *st);

extern int listing_tryget_best_alternative( listing_t *li_reference, listing_t **li_best );

#endif /* _INCLUDED_LISTING_H */
