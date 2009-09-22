/*
 * Simple configuration component API.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */


#include "config_internal.h"
#include "config_declare.h"


extern int config_init (void);
extern void config_finalise (void);

extern int config_read (void);

extern char *config_path_get (void);
extern void config_path_set (char *config_path_new);
