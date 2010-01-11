/*
 * locale(isation) module.
 *
 * Copyright (C) Matthew Turner 2009. All rights reserved.
 *
 * $Id: direntry.h 414 2009-11-28 19:17:35Z matt $
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
