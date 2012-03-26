/*
 * Simple alarm multiplexer, allowing multiple timed callbacks per process, in a
 * portable manner.
 * timer_create() et all look to be a reasonable way to do this, but they're in
 * the POSIX realtime annex, so /certain/ OSes (*cough* OS X) don't implement
 * them
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */


#include <sys/time.h>
#include <stdlib.h>
#include <signal.h>

#include "common.h"
#include "locks.h"
#include "alarms.h"
#include "queue.h"
#include "trace.h"


TRACE_DEFINE(alarms)


typedef struct _alarm_t
{
    struct timeval expiry;
    alarm_cb_t cb;
    void *cb_data;

    TAILQ_ENTRY(_alarm_t) entries;
} alarm_t;


static void sigalrm_h (int signum);
static void schedule_next (void);


/* List of alarms */
static TAILQ_HEAD(, _alarm_t) alarms_list;
static rw_lock_t *alarms_list_lock = NULL;


int alarms_init (void)
{
    alarms_trace("alarms_init()\n");

    TAILQ_INIT(&alarms_list);
    alarms_list_lock = rw_lock_new();

    signal(SIGALRM, &sigalrm_h);


    return 0;
}

void alarms_finalise (void)
{
    alarms_trace("alarms_finalise()\n");

    rw_lock_delete(alarms_list_lock);
}

extern void alarm_schedule (unsigned long seconds,
                            unsigned long useconds,
                            alarm_cb_t cb,
                            void *cb_data           )
{
    struct timeval tv_now, tv_delay, tv_alarm;
    alarm_t *alarm = (alarm_t *)calloc(1, sizeof(*alarm)), *a;


    alarms_trace("alarm_schedule(%lu.%lu)\n", seconds, useconds);

    gettimeofday(&tv_now, NULL);
    alarms_trace("time now: %lu.%lu\n", tv_now.tv_sec, tv_now.tv_usec);

    tv_delay.tv_sec  = seconds;
    tv_delay.tv_usec = useconds;

    timeradd(&tv_now, &tv_delay, &tv_alarm);
    alarms_trace("scheduling for: %lu.%lu\n", tv_alarm.tv_sec, tv_alarm.tv_usec);

    alarm->expiry  = tv_alarm;
    alarm->cb      = cb;
    alarm->cb_data = cb_data;


    rw_lock_wlock(alarms_list_lock);

    if (TAILQ_EMPTY(&alarms_list) ||
        timercmp(&tv_alarm, &TAILQ_FIRST(&alarms_list)->expiry, <))
    {
        TAILQ_INSERT_HEAD(&alarms_list, alarm, entries);

        rw_lock_wunlock(alarms_list_lock);

        schedule_next();

        rw_lock_wlock(alarms_list_lock);
    }
    else
    {
        TAILQ_FOREACH(a, &alarms_list, entries)
        {
            if (timercmp(&tv_alarm, &a->expiry, <))
            {
                TAILQ_INSERT_BEFORE(a, alarm, entries);
                break;
            }
        }
        if (!a)
        {
            TAILQ_INSERT_TAIL(&alarms_list, alarm, entries);
        }
    }

    rw_lock_wunlock(alarms_list_lock);

}

static void sigalrm_h (int signum)
{
    alarm_t *alarm;


    assert(signum == SIGALRM);

    rw_lock_rlock(alarms_list_lock);
    alarm = TAILQ_FIRST(&alarms_list);
    rw_lock_runlock(alarms_list_lock);

    (*(alarm->cb))(alarm->cb_data);

    rw_lock_wlock(alarms_list_lock);
    TAILQ_REMOVE(&alarms_list, alarm, entries);
    free(alarm);
    if (!TAILQ_EMPTY(&alarms_list))
    {
        rw_lock_wunlock(alarms_list_lock);
        schedule_next();
        rw_lock_wlock(alarms_list_lock);
    }
    rw_lock_wunlock(alarms_list_lock);
}

/* Internal function!
 * - Assumes the list is non-empty
 */
static void schedule_next (void)
{
    alarm_t *alarm;
    struct itimerval itv;
    struct timeval tv_now, tv_alarm, tv_delay;


    alarms_trace("schedule_next()\n");
    alarms_trace_indent();

    rw_lock_rlock(alarms_list_lock);
    alarm = TAILQ_FIRST(&alarms_list);
    rw_lock_runlock(alarms_list_lock);
    tv_alarm = alarm->expiry;
    alarms_trace("first alarm: %lu.%lu\n", tv_alarm.tv_sec, tv_alarm.tv_usec);

    gettimeofday(&tv_now, NULL);
    alarms_trace("time now: %lu.%lu\n", tv_now.tv_sec, tv_now.tv_usec);

    if (timercmp(&tv_alarm, &tv_now, <))
    {
        sigalrm_h(SIGALRM);
        return;
    }


    timersub(&tv_alarm, &tv_now, &tv_delay);
    alarms_trace("scheduling delay: %lu.%lu\n", tv_delay.tv_sec, tv_delay.tv_usec);

    itv.it_interval.tv_sec  = 0;
    itv.it_interval.tv_usec = 0;
    itv.it_value            = tv_delay;


    setitimer(ITIMER_REAL, &itv, NULL);

    alarms_trace_dedent();
}


#if DEBUG
/* debug functions */
void dump_alarms_list (void)
{
    alarm_t *a;
    unsigned i = 0;


    if (TAILQ_EMPTY(&alarms_list))
    {
        trce("<empty>\n");
    }
    else
    {
        TAILQ_FOREACH(a, &alarms_list, entries)
        {
            trce("[%u] expiry: %lu.%lu, cb: %p, data: %p\n", i++,
                 a->expiry.tv_sec,
                 a->expiry.tv_usec,
                 a->cb,
                 a->cb_data );
        }
    }
}
#endif
