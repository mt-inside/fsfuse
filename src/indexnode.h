/*
 * Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
 *
 * Indexnode class.
 */

#ifndef _INCLUDED_INDEXNODE_H
#define _INCLUDED_INDEXNODE_H

typedef struct _indexnode_t indexnode_t;
typedef struct _proto_indexnode_t proto_indexnode_t;


extern proto_indexnode_t *proto_indexnode_new (char *host, char *port);
/* Can't delete a proto_indexnode, can only upgrade it to a full one */

extern indexnode_t *indexnode_new (char *host, char *port, char *version, char *id);
extern indexnode_t *indexnode_from_proto (proto_indexnode_t *pin, char *version);
extern indexnode_t *indexnode_post (indexnode_t *in);
extern void indexnode_delete (indexnode_t *in);

extern char *indexnode_get_host    (indexnode_t *in);
extern char *indexnode_get_port    (indexnode_t *in);
extern void  indexnode_set_version (indexnode_t *in, char *version);
extern char *indexnode_get_version (indexnode_t *in);
extern char *indexnode_get_id      (indexnode_t *in);

extern char *proto_indexnode_make_url (
    proto_indexnode_t *pin,
    const char * const path_prefix,
    const char * const resource
);
extern char *indexnode_make_url (
    indexnode_t *in,
    const char * const path_prefix,
    const char * const resource
);

#endif /* _INCLUDED_INDEXNODE_H */
