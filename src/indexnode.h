/*
 * API for interacting with indexnodes.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#ifndef _INCLUDED_INDEXNODE_H
#define _INCLUDED_INDEXNODE_H

extern int indexnode_find (void);

extern void indexnode_parse_advert_packet (char *buf, double *version, char *port);
extern double indexnode_parse_version (char *s);
extern char *indexnode_host (void);
extern char *indexnode_port (void);
extern double indexnode_version (void);
extern int indexnode_host_is_ip (void);

#endif /* _INCLUDED_INDEXNODE_H */
