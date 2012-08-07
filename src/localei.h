/*
 * Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
 *
 * locale(isation) module.
 */

#ifndef _INCLUDED_LOCALE_H
#define _INCLUDED_LOCALE_H

#include "trace.h"


TRACE_DECLARE(locale)
#define locale_trace(...) TRACE(locale,__VA_ARGS__)
#define locale_trace_indent() TRACE_INDENT(locale)
#define locale_trace_dedent() TRACE_DEDENT(locale)


extern int locale_init (void);
extern void locale_finalise (void);

#endif /* _INCLUDED_LOCALE_H */
