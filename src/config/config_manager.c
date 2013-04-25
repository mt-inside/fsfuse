/*
 * Copyright (C) 2008-2013 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * XML file based configuration component with a simple API.
 * TODO: this isn't _reader, nor is it _data (singleton), but it is "loader" or
 * something. Knowledge of /what/ to load, e.g. paths and their order, should be
 * in fsfuse.c
 * Put config stuff in its own dir
 * respond to SIGHUP
 * - need to know which were loaded at runtime and from where so they can be a)
 *   re-loaded and b) freed
 *
 * TODO: dis should be an actor
 */

#include "common.h"

#include <pthread.h>
#include <stdlib.h>

#include "config_manager.h"
#include "config_declare.h"
#include "config_internal.h"
#include "config_loader.h"


struct _config_manager_t
{
    config_data_t **data_stack;
    size_t data_stack_len;
};


static config_manager_t *singleton = NULL;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;


static void data_push( config_manager_t *mgr, config_data_t *data );


config_manager_t *config_singleton_get( void )
{
    pthread_mutex_lock( &lock );

    if( !singleton )
    {
        singleton = calloc( 1, sizeof(*singleton) );

        data_push( singleton, config_defaults_get() );
    }

    pthread_mutex_unlock( &lock );


    return singleton;
}

void config_singleton_delete( void )
{
    pthread_mutex_lock( &lock );

    assert( singleton );

    /* TODO: go through all the (run time) data_ts and free dem */

    free( singleton );
    singleton = NULL;

    pthread_mutex_unlock( &lock );
}

static void data_push( config_manager_t *mgr, config_data_t *data )
{
    mgr->data_stack_len++;
    mgr->data_stack = realloc( mgr->data_stack, sizeof(*mgr->data_stack) * mgr->data_stack_len );

    mgr->data_stack[ mgr->data_stack_len - 1 ] = data;
}

int config_manager_add_from_file( const char *path )
{
    config_data_t *data;
    int rc = 1;


    if ( config_loader_tryread_from_file( path, &data ) )
    {
        pthread_mutex_lock( &lock );
        assert( singleton );
        data_push( singleton, data );
        pthread_mutex_unlock( &lock );

        rc = 0;
    }
    return rc;
}

void config_manager_add_from_cmdline(
    int debug_set,        int debug,
    int singlethread_set, int singlethread,
    int fg_set,           int fg
)
{
    config_data_t *data = calloc( 1, sizeof(*data) ); /* Set all "present" fields to 0 */


    data->proc_debug_present = debug_set;
    data->proc_debug = debug;
    data->proc_singlethread_present = singlethread_set;
    data->proc_singlethread = singlethread;
    data->proc_fg_present = fg_set;
    data->proc_fg = fg;

    pthread_mutex_lock( &lock );
    assert( singleton );
    data_push( singleton, data );
    pthread_mutex_unlock( &lock );
}


config_reader_t *config_get_reader( void )
{
    config_reader_t *reader = NULL;


    pthread_mutex_lock( &lock );

    if( singleton )
    {
        reader = config_reader_new( singleton->data_stack, singleton->data_stack_len );
    }

    pthread_mutex_unlock( &lock );


    return reader;
}
