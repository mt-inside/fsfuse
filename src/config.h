/*
 * Simple configuration component API.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */


/* the config */
extern char *config_alias;
extern int   config_timeout_chunk;
extern int   config_timeout_cache;
extern int   config_indexnode_autodetect;
extern char *config_indexnode_host;
extern char *config_indexnode_port;
extern int   config_attr_mode_file;
extern int   config_attr_mode_dir;
extern int   config_attr_id_uid;
extern int   config_attr_id_gid;
extern int   config_proc_fg;
extern int   config_proc_singlethread;
extern int   config_proc_debug;
extern int   config_option_cache;
extern int   config_option_progress;


extern int config_init (void);
extern void config_finalise (void);

extern int config_read (void);
