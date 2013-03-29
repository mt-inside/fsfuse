/* TODO:
 * make a pri-q
 * make an alarm_thread class
 * - to have a static singleton that can be got. This lazily creates an
 *   alarm_thread instance
 * - have a priq of alarms by next expiry
 * - pick top of queue and set timeout to hit its expiry
 * - on return from select call right cb, requeue if repeating and repeat
 * - if add something that expires before current select timeout have to kill
 *   select (e.g. with a signal to that thread or a pipe) to cause it to
 *   re-evaluate. Mustn't call the original alarm in that case.
 * when an alarm is new()d it calls alarm_thread instance to add itself
 * 
 * move alarms* to a dir
 */
struct _alarm_t
{
    alarm_cb_t cb;
    void *cb_data;
};

alarm_t *alarm_new(
    unsigned long seconds,
    int repeating,
    alarm_cb_t cb,
    void *cb_data
)
{
    alarm_t *alarm = malloc( sizeof(struct _alarm_t) );


    alarm->cb = cb;
    alarm->cb_data = cb_data;

    assert(
        !pthread_create(
            &(listener->pthread_id),
            NULL,
            &indexnodes_listen_main,
            listener->thread_args
        )
    );

    return alarm;
}

void alarm_delete(
    alarm_t alarm
)
{
}

static void *thread_main( void *ctxt )
{
    struct timeval tv;


    bzero( &tv, sizeof(tv) );
    tv.tv_sec = 1;

    while( 1 )
    {
        /* TODO: use pselect() and block all signals from getting to this
         * thread. Assert !E_AGAIN */
        select( 0, NULL, NULL, NULL, tv );

        s_cb( s_cb_data );
    }
}
