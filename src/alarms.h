/*
 * Simple alarm multiplexer API.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */


TRACE_DECLARE(alarms)


typedef void (*alarm_cb_t)(void *cb_data);


extern int alarms_init (void);
extern void alarms_finalise (void);
extern void alarm_schedule (unsigned long seconds,
                            unsigned long useconds,
                            alarm_cb_t cb,
                            void *cb_data           );
