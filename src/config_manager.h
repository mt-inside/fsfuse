/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * Simple configuration component API.
 */

#ifndef _INCLUDED_CONFIG_MANAGER_H
#define _INCLUDED_CONFIG_MANAGER_H

#include "config_declare.h"
#include "config_reader.h"


typedef struct _config_manager_t config_manager_t;


extern config_manager_t *config_singleton_get( void );
extern void config_singleton_delete( config_manager_t *singleton );

extern int config_manager_add_from_file( config_manager_t *mgr, const char *path );
extern void config_manager_add_from_cmdline( config_manager_t *mgr, int debug, int singlethread, int fg );

extern config_reader_t *config_get_reader( void );

#endif /* _INCLUDED_CONFIG_MANAGER_H */
