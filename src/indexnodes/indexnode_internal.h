/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * Indexnode class, internal methods.
 */

#ifndef _INCLUDED_INDEXNODE_INTERNAL_H
#define _INCLUDED_INDEXNODE_INTERNAL_H

#include "indexnode.h"


extern void indexnode_seen( indexnode_t * const in );
extern int indexnode_still_valid( const indexnode_t * const in );

#endif /* _INCLUDED_INDEXNODE_INTERNAL_H */
