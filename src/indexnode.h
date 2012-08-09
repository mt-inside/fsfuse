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

extern char *indexnode_get_host    (indexnode_t *in);
extern char *indexnode_get_port    (indexnode_t *in);
extern void  indexnode_set_version (indexnode_t *in, char *version);
extern char *indexnode_get_version (indexnode_t *in);
extern char *indexnode_get_id      (indexnode_t *in);

extern char *indexnode_make_url (
    indexnode_t *in,
    const char * const path_prefix,
    const char * const resource
);

#endif /* _INCLUDED_INDEXNODE_H */
