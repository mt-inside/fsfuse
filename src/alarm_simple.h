/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * Simple alarm API.
 */

#ifndef _INCLUDED_ALARMS_H
#define _INCLUDED_ALARMS_H

typedef struct _alarm_t alarm_t;
typedef void (*alarm_cb_t)( void *cb_data );


/* The callback will be called on a random thread! */
extern alarm_t *alarm_new(
    unsigned interval,
    alarm_cb_t cb,
    void *cb_data
);

extern void alarm_delete(
    alarm_t *alarm
);

#endif /* _INCLUDED_ALARMS_H */
