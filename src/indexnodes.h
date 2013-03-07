/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * Class to manage the collection of known indexnodes.
 */

#ifndef _INCLUDED_INDEXNODES_H
#define _INCLUDED_INDEXNODES_H

#include "indexnodes_iterator.h"


extern int indexnodes_init (void);
extern void indexnodes_finalise (void);

extern void indexnodes_start_listening (void);
extern void indexnodes_stop_listening (void);

extern indexnodes_list_t *indexnodes_get (void);

#endif /* _INCLUDED_INDEXNODES_H */
