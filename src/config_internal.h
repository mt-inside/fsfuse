/*
 * Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
 *
 * Internal definitions for the configuration component.
 */

typedef enum
{
    config_item_type_STRING,
    config_item_type_INTEGER,
    config_item_type_FLOAT,
    config_item_type_STRING_COLLECTION
} config_item_type_t;

typedef struct
{
    void *symbol;
    config_item_type_t type;
    char *xpath;
    int runtime; /* Has this value been updated at runtime (i.e. will it need free()ing) */
} config_item;

extern config_item config_items[];
