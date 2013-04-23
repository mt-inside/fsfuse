/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * Config reader class.
 */

#ifndef _INCLUDED_CONFIG_READER_H
#define _INCLUDED_CONFIG_READER_H

#include "config_declare.h"


typedef struct _config_reader_t config_reader_t;

#include "config_reader_functions.h" /* auto-generated at build time */


/* NOTE: these should be short-lived as they bind in the data so
 * trhey won't update when there's a SIGHUP */
extern config_reader_t *config_reader_new( config_data_t **datas, size_t datas_len );
extern void config_reader_delete( config_reader_t *config );

#endif /* _INCLUDED_CONFIG_READER_H */
