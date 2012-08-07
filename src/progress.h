/*
 * Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
 *
 * API for progress indication using curses to draw pretty things.
 */

#ifndef _INCLUDED_PROGRESS_H
#define _INCLUDED_PROGRESS_H

#include "trace.h"


TRACE_DECLARE(progress)
#define progress_trace(...) TRACE(progress,__VA_ARGS__)
#define progress_trace_indent() TRACE_INDENT(progress)
#define progress_trace_dedent() TRACE_DEDENT(progress)


extern int progress_init (void);
extern void progress_finalise (void);

extern void progress_update (const char *path,
                             unsigned len,
                             unsigned downloaded);

extern void progress_delete (const char *path);

#endif /* _INCLUDED_PROGRESS_H */
