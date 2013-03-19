#include "common.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "alarm_simple.h"


/* TODO:
 *   integration test: trivial front-end that makes a few of these and prints every they call back.
 *   make a control pipe, select on it too, switch on select return and either quit or call cb.
 */
struct _alarm_t
{
    unsigned interval;
    alarm_cb_t cb;
    void *cb_data;
    pthread_t thread;
};


static void *alarm_thread_main( void *ctxt );


alarm_t *alarm_new(
    unsigned interval,
    alarm_cb_t cb,
    void *cb_data
)
{
    alarm_t *alarm = malloc( sizeof(*alarm) );


    alarm->interval = interval;
    alarm->cb = cb;
    alarm->cb_data = cb_data;


    assert(
        !pthread_create(
            &(alarm->thread),
            NULL,
            &alarm_thread_main,
            alarm
        )
    );


    return alarm;
}

void alarm_delete(
    alarm_t *alarm
)
{
    pthread_join( alarm->thread, NULL );

    free( alarm );
}

static void *alarm_thread_main( void *ctxt )
{
    alarm_t *alarm = (alarm_t *)ctxt;
    struct timeval tv;


    while( 1 )
    {
        /* man 2 select: "consider tv undefined after select returns */
        bzero(&tv, sizeof(tv));
        tv.tv_sec = alarm->interval;

        /* TODO: pselect */
        select(0, NULL, NULL, NULL, &tv);

        alarm->cb( alarm->cb_data );
    }


    return NULL;
}
