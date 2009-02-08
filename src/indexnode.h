/*
 * API for interacting with indexnodes.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

extern int indexnode_find (void);

extern double indexnode_parse_version (char *s);
extern char *indexnode_host (void);
extern char *indexnode_port (void);
extern double indexnode_version (void);
