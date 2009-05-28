/*
 * A simple and naive config API.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#ifndef _INCLUDED_CONFIG_H
#define _INCLUDED_CONFIG_H

typedef struct /* to become a union */
{
    int int_val;
    char *str_val;
} config_item_t;

typedef enum
{
    config_key_NEXT_CHUNK_TIMEOUT,
    config_key_CACHE_EXPIRE_TIMEOUT,
    config_key_INDEXNODE_AUTODETECT,
    config_key_INDEXNODE_HOST,
    config_key_INDEXNODE_PORT,
    config_key_FILE_MODE,
    config_key_DIR_MODE,
    config_key_FS_UID,
    config_key_FS_GID,
    config_key_ALIAS,
    config_key_FOREGROUND,
    config_key_SINGLE_THREADED,
    config_key_DEBUG,
    config_key_PROGRESS,
    config_key_MOUNTPOINT
} config_key_t;


#define INT_ITEM(n) { n, "" }
#define STR_ITEM(s) { 0, s }


extern int config_init (void);
extern void config_finalise (void);
extern config_item_t config_get (config_key_t key);
extern void config_set_int (config_key_t key, int value);
extern void config_set_string (config_key_t key, char * value);

#endif /* _INCLUDED_CONFIG_H */
