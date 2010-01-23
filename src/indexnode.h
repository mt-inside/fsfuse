/*
 * API for interacting with indexnodes.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#ifndef _INCLUDED_INDEXNODE_H
#define _INCLUDED_INDEXNODE_H

extern int indexnode_init (void);
extern void indexnode_finalise (void);

extern int indexnode_find (void);

extern void indexnode_parse_version (char *buf);
extern char *indexnode_host (void);
extern char *indexnode_port (void);
extern char *indexnode_version (void);
extern int indexnode_host_is_ip (void);

#endif /* _INCLUDED_INDEXNODE_H */
