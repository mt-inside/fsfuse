/*
 * Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
 *
 * The inode -> direntry map interface.
 */

#ifndef _INCLUDED_INODE_MAP_H
#define _INCLUDED_INODE_MAP_H

#include "common.h"
#include "direntry.h"


extern void inode_map_add (direntry_t *de);
extern direntry_t *inode_map_get (ino_t inode);
extern ino_t inode_next (void);
extern void inode_map_clear (void);

#endif /* _INCLUDED_INODE_MAP_H */
