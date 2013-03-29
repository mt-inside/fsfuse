/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * Simple alarm API.
 */

#ifndef _INCLUDED_ALARMS_H
#define _INCLUDED_ALARMS_H

TRACE_DECLARE(alarms)
#define alarms_trace(...) TRACE(alarms,__VA_ARGS__)
#define alarms_trace_indent() TRACE_INDENT(alarms)
#define alarms_trace_dedent() TRACE_DEDENT(alarms)


typedef struct _alarm_t alarm_t;
typedef void (*alarm_cb_t)( void *cb_data );


extern alarm_t *alarm_new(
    unsigned long seconds,
    int repeating,
    alarm_cb_t cb,
    void *cb_data
);
/* Write me is neccessary
 * extern alarm_t alarm_new(
    time_t at,
    alarm_cb_t cb,
    void *cb_data
); */

extern void alarm_delete(
    alarm_t alarm
);

#endif /* _INCLUDED_ALARMS_H */
