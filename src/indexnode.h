/*
 * Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
 *
 * Indexnode class.
 */

#ifndef _INCLUDED_INDEXNODE_H
#define _INCLUDED_INDEXNODE_H

typedef struct _indexnode_t indexnode_t;


extern indexnode_t *indexnode_new (void);
extern void indexnode_delete (indexnode_t *in);

extern void  indexnode_set_host    (indexnode_t *in, char *host);
extern char *indexnode_get_host    (indexnode_t *in);
extern void  indexnode_set_port    (indexnode_t *in, char *port);
extern char *indexnode_get_port    (indexnode_t *in);
extern void  indexnode_set_version (indexnode_t *in, char *version);
extern char *indexnode_get_version (indexnode_t *in);
extern void  indexnode_set_id      (indexnode_t *in, char *id);
extern char *indexnode_get_id      (indexnode_t *in);

#endif /* _INCLUDED_INDEXNODE_H */
