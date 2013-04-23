/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * Functions for reading config files.
 */

#ifndef _INCLUDED_CONFIG_LOADER_H
#define _INCLUDED_CONFIG_LOADER_H

#include "config_declare.h"

extern int config_loader_tryread_from_file (const char *config_file_path, config_data_t **data);
/* TODO: shouldn't be here? */
extern void config_loader_items_free (config_data_t *data);

#endif /* _INCLUDED_LOADER_H */
