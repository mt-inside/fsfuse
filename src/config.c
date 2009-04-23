/*
 * A simple and naive config implementation with a very simple API.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#include "common.h"
#include "config.h"


static config_item_t config_array[] =
{
    INT_ITEM(5),
    INT_ITEM(10),
    INT_ITEM(1),
    STR_ITEM(""),
    STR_ITEM(""),
    INT_ITEM(0444),
    INT_ITEM(0555),
    INT_ITEM(-1),
    INT_ITEM(-1),
    STR_ITEM("Anonymous"),
    INT_ITEM(0),
    INT_ITEM(0),
    INT_ITEM(0),
    INT_ITEM(0),
    STR_ITEM("")
};

int config_init (void)
{
    /* do nothing */

    return 0;
}

void config_finalise (void)
{
    /* do nothing */
}

config_item_t config_get (config_key_t key)
{
    return config_array[key];
}

void config_set_int (config_key_t key, int value)
{
    config_array[key].int_val = value;
}
void config_set_string (config_key_t key, char *value)
{
    config_array[key].str_val = value;
}
