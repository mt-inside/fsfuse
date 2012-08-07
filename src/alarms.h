/*
 * Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
 *
 * Simple alarm multiplexer API.
 */

#ifndef _INCLUDED_ALARMS_H
#define _INCLUDED_ALARMS_H

TRACE_DECLARE(alarms)
#define alarms_trace(...) TRACE(alarms,__VA_ARGS__)
#define alarms_trace_indent() TRACE_INDENT(alarms)
#define alarms_trace_dedent() TRACE_DEDENT(alarms)


typedef void (*alarm_cb_t)(void *cb_data);


extern int alarms_init (void);
extern void alarms_finalise (void);
extern void alarm_schedule (unsigned long seconds,
                            unsigned long useconds,
                            alarm_cb_t cb,
                            void *cb_data           );

#endif /* _INCLUDED_ALARMS_H */
