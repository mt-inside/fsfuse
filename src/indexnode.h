/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * Indexnode class.
 */

#ifndef _INCLUDED_INDEXNODE_H
#define _INCLUDED_INDEXNODE_H

#include "proto_indexnode.h"


typedef struct _indexnode_t indexnode_t;


extern indexnode_t *indexnode_new (char *host, char *port, char *version, char *id);
extern indexnode_t *indexnode_from_proto (proto_indexnode_t *pin, char *version);
extern indexnode_t *indexnode_post (indexnode_t *in);
extern void indexnode_delete (indexnode_t *in);

extern char *indexnode_get_host    (indexnode_t *in);
extern char *indexnode_get_port    (indexnode_t *in);
extern char *indexnode_get_version (indexnode_t *in);
/* NULL if we can't get an ID, e.g. it's statically configured */
extern char *indexnode_get_id      (indexnode_t *in);

extern char *indexnode_make_url (
    indexnode_t *in,
    const char * const path_prefix,
    const char * const resource
);

extern void indexnode_seen (indexnode_t *in);
extern int indexnode_still_valid (indexnode_t *in);

#endif /* _INCLUDED_INDEXNODE_H */
