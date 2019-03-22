#include "common.h"

#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#include "alarm_simple.h"


/* TODO:
 *   integration test: trivial front-end that makes a few of these and prints every they call back.
 */

typedef enum
{
    alarm_control_codes_NOT_USED,
    alarm_control_codes_STOP
} alarm_control_codes_t;

struct _alarm_t
{
    unsigned interval;
    alarm_cb_t cb;
    void *cb_data;
    pthread_t thread;
    int pipe_write;
    int pipe_read;
};


static void *alarm_thread_main( void *ctxt );


alarm_t *alarm_new(
    unsigned interval,
    alarm_cb_t cb,
    void *cb_data
)
{
    int pipe_fds[2];
    alarm_t *alarm = malloc( sizeof(*alarm) );


    assert( !pipe( pipe_fds ) );
    alarm->interval = interval;
    alarm->cb = cb;
    alarm->cb_data = cb_data;
    alarm->pipe_read = pipe_fds[ 0 ];
    alarm->pipe_write = pipe_fds[ 1 ];


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
    alarm_control_codes_t msg = alarm_control_codes_STOP;


    assert( write( alarm->pipe_write, &msg, sizeof(msg) ) == sizeof(msg) );

    pthread_join( alarm->thread, NULL );

    close( alarm->pipe_write );
    close( alarm->pipe_read );


    free( alarm );
}

static void *alarm_thread_main( void *ctxt )
{
    alarm_t *alarm = (alarm_t *)ctxt;
    struct timeval tv;
    fd_set r_fds;
    int exiting = 0;
    int select_rc;
    alarm_control_codes_t msg = alarm_control_codes_NOT_USED;


    FD_ZERO(&r_fds);


    while( !exiting )
    {
        FD_SET( alarm->pipe_read, &r_fds );

        /* man 2 select: "consider tv undefined after select returns */
        bzero( &tv, sizeof(tv) );
        tv.tv_sec = alarm->interval;

        /* TODO: pselect */
        errno = 0;
        select_rc = select( alarm->pipe_read + 1, &r_fds, NULL, NULL, &tv );

        switch( select_rc )
        {
            case -1:
                trace_warn("Error waiting for alarm timeout / control signal: %s\n", strerror(errno));
                break;
            case 0:
                /* No fds active == timeout */
                alarm->cb( alarm->cb_data );
                break;
            case 1:
                if( FD_ISSET(alarm->pipe_read, &r_fds) )
                {
                    assert( read( alarm->pipe_read, &msg, sizeof(msg) ) == sizeof(msg) );
                    assert( msg == alarm_control_codes_STOP );

                    exiting = 1;
                }
                else
                {
                    assert( 0 );
                }
                break;
            default:
                assert( 0 );
        }
    }


    return NULL;
}
