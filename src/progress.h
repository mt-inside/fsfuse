/*
 * API for progress indication using curses to draw pretty things.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#ifndef _INCLUDED_PROGRESS_H
#define _INCLUDED_PROGRESS_H

#include "trace.h"


TRACE_DECLARE(progress)


extern int progress_init (void);
extern void progress_finalise (void);

extern void progress_update (const char *path,
                             unsigned len,
                             unsigned downloaded);

extern void progress_delete (const char *path);

#endif /* _INCLUDED_PROGRESS_H */
