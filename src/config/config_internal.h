/*
 * Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
 *
 * Internal definitions for the configuration component.
 */

#ifndef _INCLUDED_CONFIG_INTERNAL_H
#define _INCLUDED_CONFIG_INTERNAL_H

#include "config_declare.h"
#include "config_reader.h"


typedef enum
{
    config_item_type_STRING,
    config_item_type_INTEGER,
    config_item_type_FLOAT,
    config_item_type_STRING_COLLECTION
} config_item_type_t;

typedef struct
{
    size_t offset;
    config_item_type_t type;
    char *xpath;
} config_xml_info_item_t;

struct _config_reader_t
{
    config_data_t **datas;
    size_t datas_len;
};


extern config_xml_info_item_t config_xml_info[];


extern config_data_t *config_defaults_get( void );

/* NOTE: these should be short-lived as they bind in the data so
 * they won't update when there's a SIGHUP */
extern config_reader_t *config_reader_new( config_data_t **datas, size_t datas_len );


#endif /* _INCLUDED_CONFIG_INTERNAL_H */
