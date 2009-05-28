/*
 * statfs() declaration.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#ifndef _INCLUDED_STATFS_H
#define _INCLUDED_STATFS_H

extern int fsfuse_statfs (const char *path, struct statvfs *buf);

#endif /* _INCLUDED_STATFS_H */
