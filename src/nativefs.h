/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * API for the "nativefs"
 */

#ifndef _INCLUDED_NATIVEFS_H
#define _INCLUDED_NATIVEFS_H

#include "common.h"
/* Example session:
 * getattr( 1 )
 * stat( 1 )
 * readdir( 1 )
 * lookup( parent 1, name hello )
 * stat( 2 )
 * readdir( 1 )
 *
 * It seems inode 1 lives forever, which makes sense.
 * Other than that, it seems that lookup() is the place new nodes get
 * discovered; it's the only place that can call fuse_reply_entry(). Readdir()
 * just gets a list of names to be lookedup (although readdir can provide a fair
 * amount of the lookup()/stat() info if it wants, including an inode number,
 * though this seems to be ignored). After that, operations just happen (stat()
 * is just another boring op it seems) until the inode is forgotten */

typedef void (*nativefs_entry_found_cb_t)(
    void *ctxt,
    const char *hash,
    const char *name,
    const char *type,
    off_t size,
    unsigned long link_count,
    const char *href,
    const char *client
);

#endif /* _INCLUDED_NATIVEFS_H */
