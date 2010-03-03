/*
 * API for interacting with indexnodes.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#ifndef _INCLUDED_INDEXNODE_H
#define _INCLUDED_INDEXNODE_H

typedef struct _indexnode_t indexnode_t;


/* Global One True Indexnode singleton - for now */
extern indexnode_t *g_indexnode;


extern int indexnode_init (void);
extern void indexnode_finalise (void);

extern void indexnode_delete (indexnode_t *in);

extern indexnode_t *indexnode_find (void);

extern void indexnode_parse_version (char *buf, char *version);
extern void  indexnode_set_host    (indexnode_t *in, char *host);
extern char *indexnode_get_host    (indexnode_t *in);
extern void  indexnode_set_port    (indexnode_t *in, char *port);
extern char *indexnode_get_port    (indexnode_t *in);
extern void  indexnode_set_version (indexnode_t *in, char *version);
extern char *indexnode_get_version (indexnode_t *in);

#endif /* _INCLUDED_INDEXNODE_H */
