/*
 * Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
 *
 * The collection of known indexnodes.
 */

#ifndef _INCLUDED_INDEXNODES_H
#define _INCLUDED_INDEXNODES_H

extern int indexnodes_init (void);
extern void indexnodes_finalise (void);

extern void indexnodes_start_listening (void);
extern void indexnodes_stop_listening (void);

#endif /* _INCLUDED_INDEXNODES_H */
