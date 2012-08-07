/*
 * Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
 *
 * Simple configuration component API.
 */

#include "config_internal.h"
#include "config_declare.h"


extern int config_init (void);
extern void config_finalise (void);

extern int config_read (void);

extern char *config_path_get (void);
extern void config_path_set (char *config_path_new);
