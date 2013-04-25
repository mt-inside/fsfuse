/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * Simple configuration component API.
 */

#ifndef _INCLUDED_CONFIG_MANAGER_H
#define _INCLUDED_CONFIG_MANAGER_H

#include "config_reader.h"


typedef struct _config_manager_t config_manager_t;


extern void config_singleton_delete( void );

extern int config_manager_add_from_file( const char *path );
extern void config_manager_add_from_cmdline(
    int debug_set,        int debug,
    int singlethread_set, int singlethread,
    int fg_set,           int fg
);

extern config_reader_t *config_get_reader( void );

#endif /* _INCLUDED_CONFIG_MANAGER_H */
