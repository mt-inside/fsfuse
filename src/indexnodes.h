/*
 * The collection of known indexnodes.
 *
 * Copyright (C) Matthew Turner 2008-2011. All rights reserved.
 *
 * $Id: indexnode.h 511 2010-03-03 17:24:10Z matt $
 */

#ifndef _INCLUDED_INDEXNODES_H
#define _INCLUDED_INDEXNODES_H

extern int indexnodes_init (void);
extern void indexnodes_finalise (void);

extern indexnode_t *indexnodes_get_globalton (void);

#endif /* _INCLUDED_INDEXNODES_H */
